#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "spinlock.h"
#include "proc.h"
#include "syscall.h"
#include "defs.h"

// Fetch the uint64 at addr from the current process.
int fetchaddr(uint64 addr, uint64 *ip) {
  struct proc *p = myproc();
  if(addr >= p->sz || addr+sizeof(uint64) > p->sz) // both tests needed, in case of overflow
    return -1;
  if(copyin(p->pagetable, (char *)ip, addr, sizeof(*ip)) != 0)
    return -1;
  return 0;
}

// Fetch the nul-terminated string at addr from the current process.
int fetchstr(uint64 addr, char *buf, int max) {
  struct proc *p = myproc();
  if(copyinstr(p->pagetable, buf, addr, max) < 0)
    return -1;
  return strlen(buf);
}

static uint64 argraw(int n) {
  struct proc *p = myproc();
  switch (n) {
    case 0: return p->trapframe->a0;
    case 1: return p->trapframe->a1;
    case 2: return p->trapframe->a2;
    case 3: return p->trapframe->a3;
    case 4: return p->trapframe->a4;
    case 5: return p->trapframe->a5;
  }
  panic("argraw");
  return -1;
}

void argint(int n, int *ip) { *ip = argraw(n); }

void argaddr(int n, uint64 *ip) { *ip = argraw(n); }

int argstr(int n, char *buf, int max) {
  uint64 addr;
  argaddr(n, &addr);
  return fetchstr(addr, buf, max);
}

// Prototype cho hàm sys_trace (đổi thành uint64)
extern uint64 sys_trace(void);

// Mảng ánh xạ mã syscall từ syscall.h sang hàm xử lý
static uint64 (*syscalls[])(void) = {
  [SYS_fork]    sys_fork,
  [SYS_exit]    sys_exit,
  [SYS_wait]    sys_wait,
  [SYS_pipe]    sys_pipe,
  [SYS_read]    sys_read,
  [SYS_kill]    sys_kill,
  [SYS_exec]    sys_exec,
  [SYS_fstat]   sys_fstat,
  [SYS_chdir]   sys_chdir,
  [SYS_dup]     sys_dup,
  [SYS_getpid]  sys_getpid,
  [SYS_sbrk]    sys_sbrk,
  [SYS_sleep]   sys_sleep,
  [SYS_uptime]  sys_uptime,
  [SYS_open]    sys_open,
  [SYS_write]   sys_write,
  [SYS_mknod]   sys_mknod,
  [SYS_unlink]  sys_unlink,
  [SYS_link]    sys_link,
  [SYS_mkdir]   sys_mkdir,
  [SYS_close]   sys_close,
  [SYS_trace]   sys_trace,
};

static char *syscall_names[] = {
  "fork", "exit", "wait", "pipe", "read", "kill", "exec", "fstat",
  "chdir", "dup", "getpid", "sbrk", "sleep", "uptime", "open", "write",
  "mknod", "unlink", "link", "mkdir", "close", "trace"
};

void syscall(void) {
  int num;
  struct proc *p = myproc();

  num = p->trapframe->a7;
  if(num > 0 && num < NELEM(syscalls) && syscalls[num]) {
    int retval = syscalls[num]();
    p->trapframe->a0 = retval;

    // Nếu tiến trình này có cờ `traced` và system call này được bật trong `trace_mask`
    if (p->traced && (p->trace_mask & (1 << num))) {
      // In ra thông tin system call: ID của tiến trình, tên syscall và giá trị trả về
      printf("%d: syscall %s -> %d\n", p->pid, syscall_names[num], retval);
    }
  } else {
    printf("%d %s: unknown sys call %d\n", p->pid, p->name, num);
    p->trapframe->a0 = -1;
  }
}

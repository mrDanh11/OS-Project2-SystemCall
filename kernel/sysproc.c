#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"
#include "sysinfo.h"
#include "syscall.h"

uint64
sys_exit(void)
{
  int n;
  argint(0, &n);
  exit(n);
  return 0;  // not reached
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  return fork();
}

uint64
sys_wait(void)
{
  uint64 p;
  argaddr(0, &p);
  return wait(p);
}

uint64
sys_sbrk(void)
{
  uint64 addr;
  int n;

  argint(0, &n);
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

uint64
sys_sleep(void)
{
  int n;
  uint ticks0;

  argint(0, &n);
  if(n < 0)
    n = 0;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(killed(myproc())){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

uint64
sys_kill(void)
{
  int pid;

  argint(0, &pid);
  return kill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

uint64
sys_trace(void)
{
    int mask;
    argint(0, &mask);  // Gọi argint mà không so sánh giá trị trả về
    myproc()->tracemask = mask;
    return 0;
}

uint64
sys_sysinfo(void){
  // Khai báo các biến
  struct proc *p = myproc();
  struct sysinfo info;
  uint64 user_addr;

  // Lấy ra địa chỉ không gian người dùng
  argaddr(0, &user_addr);

  // Thu thập số byte bộ nhớ trống
  info.freemem = freemem();
  // Thu thập số tiến trình 
  info.nproc = nproc();

  // Sao chép thông tin từ kernel sang không gian người dùng
  if (copyout(p->pagetable, user_addr, (char *)&info, sizeof(info)) < 0)
    return -1;
    
  return 0;



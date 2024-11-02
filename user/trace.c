#include "kernel/types.h"
#include "user/user.h"

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(2, "Usage: trace <mask> <command> [args...]\n");
        exit(1);
    }

    int mask = atoi(argv[1]);
    trace(mask);

    // Thực thi lệnh theo dõi
    exec(argv[2], &argv[2]);
    exit(0);
}

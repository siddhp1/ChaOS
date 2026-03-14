#ifndef SYSCALL_H
#define SYSCALL_H

#include <stdint.h>

#define SYS_WRITE 0
#define SYS_EXIT 1
#define SYS_GETPID 2
#define SYS_EXECVE 3
#define SYS_FORK 4
#define SYS_READ 5

#define SYS_MAX 8

typedef long (*syscall_fn_t)(long, long, long, long, long, long);

long syscall_dispatch(long nr, long a0, long a1, long a2, long a3, long a4,
                      long a5);

void handle_el0_sync(void* frame);

#endif

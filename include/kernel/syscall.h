#ifndef SYSCALL_H
#define SYSCALL_H

#include <stdint.h>

struct syscall_frame;

void handle_el0_sync(struct syscall_frame* frame);

#endif

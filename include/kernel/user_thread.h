#ifndef USER_THREAD_H
#define USER_THREAD_H

#include <stddef.h>

#include "kernel/task.h"

#define USER_VIRT_START 0x00001000ULL
#define USER_VIRT_END 0x80000000ULL  // 2 GiB
#define USER_VIRT_ENTRY 0x00400000ULL

struct task* create_user_process(void* code, size_t code_size);
int load_user_image(struct task* t, const void* code, size_t code_size);

#endif

#ifndef USER_THREAD_H
#define USER_THREAD_H

#include <stddef.h>

#include "kernel/task.h"

#define USER_ENTRY_VA 0x00400000UL

struct task* create_user_process(void* code, size_t code_size);
int load_user_image_into_task(struct task* t, const void* code,
                              size_t code_size);

#endif

#ifndef USER_THREAD_H
#define USER_THREAD_H

#include <stddef.h>

#include "kernel/task.h"

struct task* create_user_process(void* code, size_t code_size);

#endif

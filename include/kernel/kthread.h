#ifndef KTHREAD_H
#define KTHREAD_H

#include "task.h"

struct task* kthread_create(void (*fn)(void*), void* arg);

#endif

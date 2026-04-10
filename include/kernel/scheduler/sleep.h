#ifndef SLEEP_H
#define SLEEP_H

#include <stdint.h>

#include "kernel/task.h"

extern struct task* sleep_queue;

void task_sleep(uint64_t ticks, struct task* task);
void check_sleeping_tasks(void);

#endif

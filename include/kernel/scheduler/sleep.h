#ifndef SLEEP_H
#define SLEEP_H

#include <stdint.h>

#include "kernel/task.h"

extern struct task* sleep_queue;

void enqueue_sleep_task(struct task* task);
void dequeue_sleep_task(struct task* task);
void task_sleep(uint64_t ticks, struct task* task);
void check_sleeping_tasks(void);

#endif

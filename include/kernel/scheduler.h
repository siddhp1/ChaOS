#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "kernel/task.h"

extern volatile uint64_t system_tick;

extern struct task* ready_queue;
extern struct task* current_task;

void scheduler_init(void);
void enqueue_task(struct task* task);
void dequeue_task(struct task* task);
void schedule(void);

#endif

#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <stdbool.h>

#include "kernel/task.h"

extern struct task* ready_queue;
extern struct task* current_task;
extern volatile uint64_t system_tick;

void yield(void);
void scheduler_init(void);
void scheduler_tick(void);
void enqueue_task(struct task* task);
void dequeue_task(struct task* task);
uint64_t schedule(uint64_t irq_sp);

#endif

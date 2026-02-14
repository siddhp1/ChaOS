#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <stdbool.h>

#include "kernel/task.h"

// Number of hardware timer interrupts before scheduling
#define DEFAULT_TIME_SLICE 5

extern struct task* ready_queue;
extern struct task* current_task;
extern volatile bool need_schedule;

void scheduler_init(void);
void scheduler_tick(void);
void enqueue_task(struct task* task);
void dequeue_task(struct task* task);
void schedule(void);

#endif

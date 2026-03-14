#ifndef WAIT_H
#define WAIT_H

#include "kernel/scheduler/scheduler.h"
#include "kernel/task.h"

extern struct task* wait_queue;

void enqueue_wait_task(struct task* task);
void dequeue_wait_task(void);
void wait_event(struct task* task);
void wake_up(void);
void wake_up_all(void);

#endif

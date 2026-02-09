#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "kernel/task.h"

extern task_t* ready_queue;
extern task_t* current_task;

void scheduler_init(void);
void enqueue_task(task_t* task);
void dequeue_task(task_t* task);
task_t* get_next_task(void);
void scheduler(void);

#endif

#ifndef SLEEP_H
#define SLEEP_H

#include <stdint.h>

#include "kernel/task.h"

struct sleep_queue {
  struct task* head;
};

extern struct sleep_queue sleep_queue;

void sleep_queue_init(struct sleep_queue* sq);
void enqueue_sleep_task(struct sleep_queue* sq, struct task* task);
void dequeue_sleep_task(struct sleep_queue* sq, struct task* task);
void task_sleep(uint64_t ticks, struct sleep_queue* sq,
                struct task* current_task);
void check_sleeping_tasks(struct sleep_queue* sq);

#endif

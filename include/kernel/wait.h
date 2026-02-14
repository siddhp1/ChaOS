#ifndef WAIT_H
#define WAIT_H

#include "kernel/scheduler.h"
#include "kernel/task.h"

struct wait_queue {
  struct task* head;
};

void wait_queue_init(struct wait_queue* wq);
void enqueue_wait_task(struct wait_queue* wq, struct task* current_task);
void dequeue_wait_task(struct wait_queue* wq);
void wait_event(struct wait_queue* wq, struct task* current_task);
void wake_up(struct wait_queue* wq);
void wake_up_all(struct wait_queue* wq);

#endif

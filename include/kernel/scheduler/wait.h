#ifndef WAIT_H
#define WAIT_H

struct task;

struct wait_queue {
  struct task* head;
  struct task* tail;
};

void wait_queue_init(struct wait_queue* wq);
void task_wait(struct wait_queue* wq, struct task* task);
void unwait(struct wait_queue* wq);
void unwait_all(struct wait_queue* wq);

#endif

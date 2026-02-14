#include "kernel/wait.h"

#include <stddef.h>

#include "kernel/irq.h"

void wait_queue_init(struct wait_queue* wq) { wq->head = NULL; }

void enqueue_wait_task(struct wait_queue* wq, struct task* current_task) {
  current_task->next = NULL;

  if (!wq->head) {
    wq->head = current_task;
  } else {
    struct task* last_task = wq->head;
    while (last_task->next) {
      last_task = last_task->next;
    }
    last_task->next = current_task;
  }
}

void dequeue_wait_task(struct wait_queue* wq) {
  if (!wq->head) return;

  struct task* current_task = wq->head;
  current_task->state = TASK_READY;
  wq->head = wq->head->next;
  current_task->next = NULL;
}

void wait_event(struct wait_queue* wq, struct task* current_task) {
  irq_disable();

  if (!current_task) {
    irq_enable();
    return;
  }

  current_task->state = TASK_BLOCKED;
  enqueue_wait_task(wq, current_task);

  irq_enable();
  schedule();
}

void wake_up(struct wait_queue* wq) {
  irq_disable();

  if (!wq->head) {
    irq_enable();
    return;
  }

  struct task* woken = wq->head;
  dequeue_wait_task(wq);
  enqueue_task(woken);

  irq_enable();
}

void wake_up_all(struct wait_queue* wq) {
  irq_disable();

  while (wq->head) {
    struct task* woken = wq->head;
    dequeue_wait_task(wq);
    enqueue_task(woken);
  }

  irq_enable();
}

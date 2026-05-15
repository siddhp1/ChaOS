#include "kernel/scheduler/wait.h"

#include <stddef.h>

#include "kernel/irq.h"
#include "kernel/scheduler/scheduler.h"

static void enqueue_wait_task(struct wait_queue* wq, struct task* task) {
  if (!wq || !task) {
    return;
  }

  task->next = NULL;

  if (!wq->head) {
    wq->head = task;
    wq->tail = task;
    return;
  }

  wq->tail->next = task;
  wq->tail = task;
}

static struct task* dequeue_wait_task(struct wait_queue* wq) {
  if (!wq || !wq->head) return NULL;

  struct task* task = wq->head;

  wq->head = wq->head->next;

  if (!wq->head) {
    wq->tail = NULL;
  }

  task->next = NULL;
  return task;
}

void wait_queue_init(struct wait_queue* wq) {
  if (!wq) {
    return;
  }

  wq->head = NULL;
  wq->tail = NULL;
}

void task_wait(struct wait_queue* wq, struct task* task) {
  if (!wq || !task) return;

  uint64_t daif = irq_save();

  if (task->state != TASK_BLOCKED) {
    task->state = TASK_BLOCKED;
    enqueue_wait_task(wq, task);
  }

  irq_restore(daif);
  yield();
}

void unwait(struct wait_queue* wq) {
  if (!wq) return;

  uint64_t daif = irq_save();

  struct task* task = dequeue_wait_task(wq);

  if (!task) {
    irq_restore(daif);
    return;
  }
  enqueue_task(task);

  irq_restore(daif);
}

void unwait_all(struct wait_queue* wq) {
  if (!wq) return;

  uint64_t daif = irq_save();

  struct task* task;
  while ((task = dequeue_wait_task(wq)) != NULL) {
    enqueue_task(task);
  }

  irq_restore(daif);
}

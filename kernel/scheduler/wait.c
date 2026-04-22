#include "kernel/scheduler/wait.h"

#include <stddef.h>

#include "kernel/irq.h"
#include "kernel/scheduler/scheduler.h"

struct task* wait_queue = NULL;

void enqueue_wait_task(struct task* task) {
  if (!task) {
    return;
  }

  task->next = NULL;

  if (!wait_queue) {
    wait_queue = task;
    return;
  }

  struct task* last_task = wait_queue;
  while (last_task->next) {
    last_task = last_task->next;
  }
  last_task->next = task;
  task->next = NULL;
}

void dequeue_wait_task(void) {
  if (!wait_queue) return;

  struct task* current_task = wait_queue;
  wait_queue = wait_queue->next;
  current_task->next = NULL;
}

void task_wait(struct task* task) {
  irq_disable();

  if (!task) {
    irq_enable();
    return;
  }

  task->state = TASK_BLOCKED;
  enqueue_wait_task(task);

  irq_enable();
  yield();
}

void unwait(void) {
  irq_disable();

  if (!wait_queue) {
    irq_enable();
    return;
  }

  struct task* task = wait_queue;
  dequeue_wait_task();
  enqueue_task(task);

  irq_enable();
}

void unwait_all(void) {
  irq_disable();

  while (wait_queue) {
    struct task* task = wait_queue;
    dequeue_wait_task();
    enqueue_task(task);
  }

  irq_enable();
}

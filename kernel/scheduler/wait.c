#include "kernel/scheduler/wait.h"

#include <stddef.h>

#include "kernel/irq.h"

struct task* wait_queue = NULL;

void enqueue_wait_task(struct task* task) {
  task->next = NULL;

  if (!wait_queue) {
    wait_queue = task;
  } else {
    struct task* last_task = wait_queue;
    while (last_task->next) {
      last_task = last_task->next;
    }
    last_task->next = task;
  }
}

void dequeue_wait_task(void) {
  if (!wait_queue) return;

  struct task* current_task = wait_queue;
  current_task->state = TASK_READY;
  wait_queue = wait_queue->next;
  current_task->next = NULL;
}

void wait_event(struct task* task) {
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

void wake_up(void) {
  irq_disable();

  if (!wait_queue) {
    irq_enable();
    return;
  }

  struct task* woken = wait_queue;
  dequeue_wait_task();
  enqueue_task(woken);

  irq_enable();
}

void wake_up_all(void) {
  irq_disable();

  while (wait_queue) {
    struct task* woken = wait_queue;
    dequeue_wait_task();
    enqueue_task(woken);
  }

  irq_enable();
}

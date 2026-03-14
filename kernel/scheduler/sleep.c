#include "kernel/scheduler/sleep.h"

#include <stddef.h>
#include <stdint.h>

#include "kernel/irq.h"
#include "kernel/scheduler/scheduler.h"

struct task* sleep_queue = NULL;

void enqueue_sleep_task(struct task* task) {
  task->next = NULL;

  if (!sleep_queue) {
    sleep_queue = task;
  } else {
    struct task* last_task = sleep_queue;
    while (last_task->next) {
      last_task = last_task->next;
    }
    last_task->next = task;
  }
}

void dequeue_sleep_task(struct task* task) {
  if (!sleep_queue || !task) return;

  if (sleep_queue == task) {
    sleep_queue = sleep_queue->next;
    task->next = NULL;
    return;
  }

  struct task* prev = sleep_queue;
  struct task* curr = sleep_queue->next;
  while (curr) {
    if (curr == task) {
      prev->next = curr->next;
      curr->next = NULL;
      return;
    }
    prev = curr;
    curr = curr->next;
  }
}

void task_sleep(uint64_t ticks, struct task* task) {
  irq_disable();

  if (!task) {
    irq_enable();
    return;
  }

  task->wakeup_tick = system_tick + ticks;
  task->state = TASK_SLEEPING;
  enqueue_sleep_task(task);

  irq_enable();

  schedule();
}

void check_sleeping_tasks(void) {
  struct task* task = sleep_queue;
  while (task) {
    struct task* next_task = task->next;
    if (task->state == TASK_SLEEPING && task->wakeup_tick <= system_tick) {
      dequeue_sleep_task(task);
      task->state = TASK_READY;
      enqueue_task(task);
    }
    task = next_task;
  }
}

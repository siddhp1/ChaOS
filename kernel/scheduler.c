#include "kernel/scheduler.h"

#include <stdbool.h>
#include <stddef.h>

#include "kernel/printk.h"
#include "kernel/task.h"

extern void context_switch(struct cpu_context* a, struct cpu_context* b);

struct task* ready_queue = NULL;
struct task* current_task = NULL;
volatile bool need_schedule = false;

void scheduler_init(void) {
}

void scheduler_tick(void) {
  if (!current_task) {
    return;
  }

  current_task->time_slice--;

  if (current_task->time_slice <= 0) {
    current_task->time_slice = DEFAULT_TIME_SLICE;
    need_schedule = true;
  }
}

void enqueue_task(struct task* task) {
  if (!ready_queue) {
    ready_queue = task;
  } else {
    struct task* last_task = ready_queue;
    while (last_task->next) {
      last_task = last_task->next;
    }
    last_task->next = task;
  }
}

void dequeue_task(struct task* task) {
  if (!ready_queue || !task) return;

  if (ready_queue == task) {
    ready_queue = ready_queue->next;
    task->next = NULL;
    return;
  }

  struct task* prev = ready_queue;
  struct task* curr = ready_queue->next;
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

void schedule(void) {
  if (!ready_queue) return;
  if (current_task) {
    current_task->state = TASK_READY;
    dequeue_task(current_task);
    enqueue_task(current_task);
  }
  struct task* prev = current_task;

  current_task = ready_queue;
  if (current_task) {
    current_task->state = TASK_RUNNING;
  }

  context_switch(&prev->context, &current_task->context);
}

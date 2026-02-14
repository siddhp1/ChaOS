#include "kernel/scheduler.h"

#include <stddef.h>
#include <stdint.h>

#include "kernel/printk.h"
#include "kernel/task.h"

extern void context_switch(struct cpu_context* a, struct cpu_context* b);

volatile uint64_t system_tick = 0;

struct task* ready_queue = NULL;
struct task* current_task = NULL;

void scheduler_init(void) {
  ready_queue = NULL;
  current_task = NULL;
}

void enqueue_task(struct task* task) {
  task->next = NULL;

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
    if (current_task->state == TASK_RUNNING) {
      current_task->state = TASK_READY;
    }
    if (current_task->state == TASK_READY) {
      dequeue_task(current_task);
      enqueue_task(current_task);
    }
  }
  struct task* prev = current_task;

  current_task = ready_queue;
  if (current_task) {
    current_task->state = TASK_RUNNING;
  }

  context_switch(&prev->context, &current_task->context);
}

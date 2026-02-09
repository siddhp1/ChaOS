#include "kernel/scheduler.h"

#include <stddef.h>

#include "kernel/task.h"

task_t* ready_queue = NULL;
task_t* current_task = NULL;

void scheduler_init(void) {
  ready_queue = NULL;
  current_task = NULL;
}

void enqueue_task(task_t* task) {
  if (!ready_queue) {
    ready_queue = task;
  } else {
    task_t* last_task = ready_queue;
    while (last_task->next) {
      last_task = last_task->next;
    }
    last_task->next = task;
  }
}

void dequeue_task(task_t* task) {
  if (!ready_queue || !task) return;

  if (ready_queue == task) {
    ready_queue = ready_queue->next;
    task->next = NULL;
    return;
  }

  task_t* prev = ready_queue;
  task_t* curr = ready_queue->next;
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

task_t* get_next_task(void) { return ready_queue; }

void scheduler(void) {
  if (!ready_queue) return;
  if (current_task) {
    current_task->state = TASK_READY;
    dequeue_task(current_task);
    enqueue_task(current_task);
  }
  current_task = get_next_task();
  if (current_task) {
    current_task->state = TASK_RUNNING;
  }
}

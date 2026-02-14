#include "kernel/sleep.h"

#include <stddef.h>
#include <stdint.h>

#include "kernel/irq.h"
#include "kernel/scheduler.h"

struct sleep_queue sleep_queue = {.head = NULL};

void sleep_queue_init(struct sleep_queue* sq) { sq->head = NULL; }

void enqueue_sleep_task(struct sleep_queue* sq, struct task* current_task) {
  current_task->next = NULL;

  if (!sq->head) {
    sq->head = current_task;
  } else {
    struct task* last_task = sq->head;
    while (last_task->next) {
      last_task = last_task->next;
    }
    last_task->next = current_task;
  }
}

void dequeue_sleep_task(struct sleep_queue* sq, struct task* task) {
  if (!sq->head || !task) return;

  if (sq->head == task) {
    sq->head = sq->head->next;
    task->next = NULL;
    return;
  }

  struct task* prev = sq->head;
  struct task* curr = sq->head->next;
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

void task_sleep(uint64_t ticks, struct sleep_queue* sq,
                struct task* current_task) {
  irq_disable();

  if (!current_task) {
    irq_enable();
    return;
  }

  current_task->wakeup_tick = system_tick + ticks;
  current_task->state = TASK_SLEEPING;
  enqueue_sleep_task(sq, current_task);

  irq_enable();

  schedule();
}

void check_sleeping_tasks(struct sleep_queue* sq) {
  struct task* task = sq->head;
  while (task) {
    struct task* next_task = task->next;
    if (task->state == TASK_SLEEPING && task->wakeup_tick <= system_tick) {
      dequeue_sleep_task(sq, task);
      task->state = TASK_READY;
      enqueue_task(task);
    }
    task = next_task;
  }
}

#include "kernel/scheduler/scheduler.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "kernel/irq.h"
#include "kernel/kthread.h"
#include "kernel/scheduler/reaper.h"
#include "kernel/scheduler/sleep.h"
#include "kernel/string.h"
#include "kernel/task.h"
#include "mm/mmu.h"
#include "mm/pgtable.h"

volatile uint64_t system_tick = 0;

struct task* ready_queue = NULL;
struct task* current_task = NULL;
volatile bool need_schedule = false;

static struct task boot_task;
static struct task* idle_task = NULL;

void scheduler_init(void) {
  memset(&boot_task, 0, sizeof(boot_task));

  boot_task.pid = 0;
  boot_task.state = TASK_RUNNING;
  boot_task.mode = TASK_MODE_KERNEL;
  boot_task.time_slice = DEFAULT_TIME_SLICE;
  boot_task.ttbr0 = 0;

  idle_task = &boot_task;
  current_task = idle_task;
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

  check_sleeping_tasks();

  if (system_tick % REAP_TICKS == 0) {
    reap_zombies();
  }
}

void enqueue_task(struct task* task) {
  if (!task) {
    return;
  }

  task->state = TASK_READY;
  task->next = NULL;

  if (!ready_queue) {
    ready_queue = task;
    return;
  }

  struct task* last_task = ready_queue;
  while (last_task->next) {
    last_task = last_task->next;
  }
  last_task->next = task;
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

struct task* get_next_task(void) {
  if (!idle_task) {
    return NULL;
  }

  if (current_task && current_task->state == TASK_RUNNING) {
    if (current_task != idle_task) {
      enqueue_task(current_task);
    }
  }

  struct task* next = ready_queue;
  if (next) {
    dequeue_task(next);
  } else {
    next = idle_task;
  }

  next->state = TASK_RUNNING;
  current_task = next;
  return next;
}

void yield(void) {
  need_schedule = true;
  // TODO: Trigger a reschedule immediately
  asm volatile("WFI");
}

uint64_t schedule(uint64_t irq_sp) {
  if (current_task) {
    current_task->irq_sp = irq_sp;
  }

  if (!need_schedule) {
    return irq_sp;
  }
  need_schedule = false;

  struct task* prev = current_task;
  struct task* next = get_next_task();
  if (!next || next == prev) {
    return irq_sp;
  }

  if (next->mode == TASK_MODE_USER) {
    set_ttbr0(next->ttbr0);
  }

  return next->irq_sp;
}

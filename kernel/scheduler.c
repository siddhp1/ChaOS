#include "kernel/scheduler.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "kernel/irq.h"
#include "kernel/kthread.h"
#include "kernel/string.h"
#include "kernel/task.h"
#include "kernel/trap.h"
#include "mm/mmu.h"

// PSTATE bits
#define SPSR_MODE_EL1H 0x5u
#define SPSR_MODE_EL0T 0x0u

volatile uint64_t system_tick = 0;

struct task* ready_queue = NULL;
struct task* current_task = NULL;
volatile bool need_schedule = false;

static struct task* idle_task = NULL;

static inline void build_first_frame(struct task* task,
                                     struct trapframe* trapframe) {
  memset(trapframe, 0, sizeof(*trapframe));

  if (task_is_user(task)) {
    trapframe->elr_el1 = task->user_entry;
    trapframe->sp_el0 = task->user_sp;
    trapframe->spsr_el1 = SPSR_MODE_EL0T;
  } else {
    // Kernel threads start at context.lr (kthread_entry)
    trapframe->elr_el1 = task->context.lr;
    trapframe->spsr_el1 = SPSR_MODE_EL1H;
  }
}

void idle_thread(void* arg) { while (1); }

void scheduler_init(void) {
  idle_task = kthread_create(idle_thread, NULL);

  idle_task->state = TASK_RUNNING;
  current_task = idle_task;

  need_schedule = false;
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
  if (!task) {
    return;
  }

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

  // If the current task is still running, make it runnable again
  if (current_task && current_task->state == TASK_RUNNING) {
    current_task->state = TASK_READY;
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

void schedule(void) {
  need_schedule = true;
  asm volatile("WFI");
}

// IRQ-return scheduling
uint64_t scheduler_irq_exit(uint64_t irq_sp) {
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

  mmu_switch_ttbr0(next->ttbr0);

  if (next->irq_sp == 0) {
    uint64_t frame_sp = next->context.sp - sizeof(struct trapframe);
    struct trapframe* trapframe = (struct trapframe*)frame_sp;

    // TODO: Consider condensing
    build_first_frame(next, trapframe);
    next->irq_sp = frame_sp;
  }

  return next->irq_sp;
}

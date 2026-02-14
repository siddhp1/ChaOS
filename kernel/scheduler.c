#include "kernel/scheduler.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "kernel/irq.h"
#include "kernel/kthread.h"
#include "kernel/printk.h"
#include "kernel/string.h"
#include "kernel/task.h"

// TODO: Sync with vectors.s
#define IRQ_FRAME_SIZE (16 * 17)
#define IRQ_OFF_ELR_SPSR (16 * 16)
#define SPSR_EL1H (0x5)  // EL1h, interrupts unmasked

extern void context_switch(struct cpu_context* a, struct cpu_context* b);

volatile uint64_t system_tick = 0;

struct task* ready_queue = NULL;
struct task* current_task = NULL;
volatile bool need_schedule = false;

// TODO: Remove idle task skip
static struct task* idle_task = NULL;

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
  if (!ready_queue) {
    return NULL;
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
  // TODO: Remove idle task skip
  // Currently prevents the idle thread from getting enqueued
  if (prev && prev->state == TASK_RUNNING && prev != idle_task) {
    prev->state = TASK_READY;
    dequeue_task(prev);
    enqueue_task(prev);
  }

  struct task* next = ready_queue;
  dequeue_task(next);
  next->state = TASK_RUNNING;
  current_task = next;

  return next;
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

  struct task* next = get_next_task();
  if (!next) {
    return irq_sp;
  }

  if (next->irq_sp == 0) {
    // Create a synthetic IRQ frame on the new task's stack
    uint64_t frame_sp = next->context.sp - IRQ_FRAME_SIZE;

    memset((void*)frame_sp, 0, IRQ_FRAME_SIZE);

    volatile uint64_t* elr_spsr =
        (volatile uint64_t*)(frame_sp + IRQ_OFF_ELR_SPSR);
    elr_spsr[0] = next->context.lr;
    elr_spsr[1] = SPSR_EL1H;

    next->irq_sp = frame_sp;
  }

  return next->irq_sp;
}

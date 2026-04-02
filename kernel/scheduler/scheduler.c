#include "kernel/scheduler/scheduler.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "kernel/irq.h"
#include "kernel/kthread.h"
#include "kernel/scheduler/reaper.h"
#include "kernel/string.h"
#include "kernel/task.h"
#include "mm/mmu.h"
#include "mm/pgtable.h"

// TODO: Sync with vectors.S
#define IRQ_OFF_ELR_SPSR (16 * 16)
#define IRQ_OFF_USER_SP (16 * 17)
#define IRQ_FRAME_SIZE (16 * 18)
#define SPSR_EL1H (0x5)  // EL1h, interrupts unmasked

#define USER_STACK_TOP 0x0000000080000000ULL  // 2 GiB

#define REAP_TICKS 100

volatile uint64_t system_tick = 0;

struct task* ready_queue = NULL;
struct task* current_task = NULL;
volatile bool need_schedule = false;

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

  if (system_tick % REAP_TICKS == 0) {
    reap_zombies();
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

uint64_t scheduler_irq_exit(uint64_t irq_sp) {
  if (current_task) {
    current_task->irq_sp = irq_sp;
  }

  if (!need_schedule) {
    return irq_sp;
  }
  need_schedule = false;

  struct task* prev = current_task;

  if (prev && prev->mode == TASK_MODE_USER) {
    uint64_t sp_el0;
    asm volatile("mrs %0, SP_EL0" : "=r"(sp_el0));
    prev->sp_el0 = sp_el0;
  }

  struct task* next = get_next_task();
  if (!next || next == prev) {
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

  if (next->mode == TASK_MODE_USER) {
    asm volatile("msr SP_EL0, %0" ::"r"(next->sp_el0) : "memory");
    set_ttbr0(next->ttbr0);
  }

  return next->irq_sp;
}

void save_user_sp_el0(uint64_t irq_sp) {
  if (!current_task || current_task->mode != TASK_MODE_USER) {
    return;
  }

  uint64_t* user_sp_ptr = (uint64_t*)(irq_sp + IRQ_OFF_USER_SP);
  current_task->sp_el0 = *user_sp_ptr;
}

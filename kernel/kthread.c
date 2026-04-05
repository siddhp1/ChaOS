#include "kernel/kthread.h"

#include <stddef.h>

#include "kernel/irq.h"
#include "kernel/pid.h"
#include "kernel/scheduler/scheduler.h"
#include "kernel/task.h"

#define KSTACK_SIZE 4096

static void kthread_entry(void) {
  irq_enable();

  current_task->fn(current_task->arg);

  irq_disable();
  current_task->state = TASK_ZOMBIE;
  need_schedule = true;
  irq_enable();

  while (1) {
    asm volatile("WFI");
  }
}

struct task* kthread_create(void (*fn)(void*), void* arg) {
  struct task* t = alloc_task();
  if (!t) {
    return NULL;
  }

  void* stack_base = alloc_stack();
  if (!stack_base) {
    return NULL;
  }

  uint64_t stack_top = (uint64_t)stack_base + KSTACK_SIZE;

  t->context.sp = stack_top;
  t->context.lr = (uint64_t)kthread_entry;

  t->state = TASK_READY;
  t->pid = pid_alloc();

  t->fn = fn;
  t->arg = arg;

  t->stack = (uint64_t)stack_base;

  t->irq_sp = (uint64_t)NULL;

  t->time_slice = DEFAULT_TIME_SLICE;

  t->mode = TASK_MODE_KERNEL;
  t->ttbr0 = 0;

  t->next = NULL;

  create_irq_frame(t);

  enqueue_task(t);

  return t;
}

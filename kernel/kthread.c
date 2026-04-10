#include "kernel/kthread.h"

#include <stddef.h>

#include "kernel/irq.h"
#include "kernel/pid.h"
#include "kernel/scheduler/reaper.h"
#include "kernel/scheduler/scheduler.h"
#include "kernel/task.h"

static void kthread_entry(void) {
  current_task->fn(current_task->arg);

  task_zombie(current_task);

  yield();
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

  uintptr_t stack_top = (uintptr_t)stack_base + KSTACK_SIZE;

  t->pid = pid_alloc();

  t->fn = fn;
  t->arg = arg;

  t->stack = (uintptr_t)stack_base;

  t->irq_sp = (uintptr_t)NULL;

  t->time_slice = DEFAULT_TIME_SLICE;

  t->mode = TASK_MODE_KERNEL;

  t->next = NULL;

  create_irq_frame(t, stack_top, (uintptr_t)kthread_entry, 0);

  enqueue_task(t);

  return t;
}

#include "kernel/kthread.h"

#include <stddef.h>

#include "kernel/irq.h"
#include "kernel/scheduler.h"
#include "kernel/task.h"
#include "pid.h"
#include "task_internal.h"

#define KSTACK_SIZE 4096

static void kthread_entry(void) {
  irq_enable();

  current_task->fn(current_task->arg);

  while (1) {
    schedule();
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

  t->next = NULL;

  enqueue_task(t);

  return t;
}

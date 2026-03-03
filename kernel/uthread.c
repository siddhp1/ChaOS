#include <stddef.h>
#include <stdint.h>

#include "kernel/irq.h"
#include "kernel/scheduler.h"
#include "kernel/string.h"
#include "kernel/task.h"
#include "kernel/vm.h"
#include "mm/page.h"
#include "pid.h"
#include "task_internal.h"

#define KSTACK_SIZE 4096

int32_t utask_create(uint64_t user_entry, uint64_t user_sp, uint64_t ttbr0) {
  struct task* t = alloc_task();
  if (!t) return -1;

  void* stack_base = alloc_stack();
  if (!stack_base) return -1;

  memset(&t->context, 0, sizeof(t->context));

  t->context.sp = (uint64_t)stack_base + KSTACK_SIZE;
  t->context.lr = 0;  // first return path is synthesized in scheduler

  t->state = TASK_READY;
  t->pid = pid_alloc();

  t->mode = TASK_MODE_USER;
  t->fn = NULL;
  t->arg = NULL;

  t->stack = (uint64_t)stack_base;
  t->irq_sp = 0;  // force first-frame build in scheduler_irq_exit

  t->time_slice = DEFAULT_TIME_SLICE;
  t->wakeup_tick = 0;

  t->user_entry = user_entry;
  t->user_sp = user_sp;

  t->ttbr0 = ttbr0;
  t->user_page_count = 0;

  t->parent = NULL;
  t->next = NULL;

  enqueue_task(t);
  return t->pid;
}

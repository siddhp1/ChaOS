#include <stddef.h>

#include "kernel/initramfs.h"
#include "kernel/irq.h"
#include "kernel/printk.h"
#include "kernel/scheduler/scheduler.h"
#include "mm/mmu.h"
#include "mm/pgtable.h"
#include "syscall_handlers.h"

long sys_exit(long status, long a1, long a2, long a3, long a4, long a5) {
  (void)a1;
  (void)a2;
  (void)a3;
  (void)a4;
  (void)a5;

  current_task->exit_status = (int32_t)status;
  printk("User process exited with status: %ld\n", status);

  irq_disable();

  struct task* child = current_task->first_child;
  while (child) {
    struct task* next = child->sibling_next;
    child->parent = task_init;
    child->sibling_next = task_init->first_child;
    task_init->first_child = child;
    child = next;
  }
  current_task->first_child = NULL;

  if (task_init && task_init->state == TASK_WAIT_CHILD) {
    task_init->state = TASK_READY;
    enqueue_task(task_init);
  }

  struct task* parent = current_task->parent;
  if (parent && parent->state == TASK_WAIT_CHILD) {
    parent->state = TASK_READY;
    enqueue_task(parent);
  }

  current_task->state = TASK_ZOMBIE;

  irq_enable();
  yield();

  return 0;
}

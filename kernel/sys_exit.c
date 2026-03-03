#include "kernel/scheduler.h"
#include "kernel/task.h"
#include "kernel/vm.h"

long sys_exit(long status, long a1, long a2, long a3, long a4, long a5) {
  (void)status;
  (void)a1;
  (void)a2;
  (void)a3;
  (void)a4;
  (void)a5;

  if (current_task->ttbr0) {
    vm_destroy_user(current_task);
  }

  current_task->state = TASK_ZOMBIE;
  need_schedule = true;

  while (1) asm volatile("wfi");
}

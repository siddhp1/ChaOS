#include "kernel/scheduler.h"
#include "kernel/task.h"
#include "kernel/vm.h"
#include "mm/mmu.h"

long sys_exit(long status, long a1, long a2, long a3, long a4, long a5) {
  (void)status;
  (void)a1;
  (void)a2;
  (void)a3;
  (void)a4;
  (void)a5;

  // TODO: Add process exit status
  // current_task->exit_status = status;

  // Switch TTBR0 to kernel page table before destroying user memory
  // to prevent TTBR0 from pointing to freed memory
  if (current_task->ttbr0) {
    mmu_switch_ttbr0(mmu_kernel_ttbr0());
    vm_destroy_user(current_task);
  }

  current_task->state = TASK_ZOMBIE;
  schedule();

  return 0;
}

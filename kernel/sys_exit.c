#include <stddef.h>

#include "kernel/printk.h"
#include "kernel/scheduler.h"
#include "mm/pgtable.h"

long sys_exit(long status, long a1, long a2, long a3, long a4, long a5) {
  (void)status;
  (void)a1;
  (void)a2;
  (void)a3;
  (void)a4;
  (void)a5;

  printk("User process exited with status: ");
  printk_hex_u64(status);
  printk("\n");

  // TODO: Add process exit status
  // current_task->exit_status = status;

  // Set ttbr0 to 0
  switch_user_pgd(NULL);

  current_task->state = TASK_ZOMBIE;
  schedule();

  return 0;
}

#include <stddef.h>

#include "kernel/printk.h"
#include "kernel/scheduler/reaper.h"
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

  current_task->exit_status = status;
  printk("User process exited with status: %ld\n", status);

  task_zombie(current_task);
  yield();

  return 0;
}

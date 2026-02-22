#include "kernel/scheduler.h"

long sys_exit(long status, long a1, long a2, long a3, long a4, long a5) {
  // TOOD: Add process exit status
  // current_task->exit_status = status;

  schedule();

  while (1);  // Should never return
}

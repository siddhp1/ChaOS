#include "kernel/scheduler.h"
#include "kernel/task.h"

long sys_exit(long status, long a1, long a2, long a3, long a4, long a5) {
  // TODO: Add process exit status
  // current_task->exit_status = status;

  current_task->state = TASK_ZOMBIE;
  schedule();

  return 0;
}

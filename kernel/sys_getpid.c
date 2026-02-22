#include "kernel/scheduler.h"

long sys_getpid(long a0, long a1, long a2, long a3, long a4, long a5) {
  return current_task->pid;
}

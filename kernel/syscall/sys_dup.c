#include "fs/file.h"
#include "kernel/scheduler/scheduler.h"

long sys_dup(long old_fd, long a1, long a2, long a3, long a4, long a5) {
  (void)a1;
  (void)a2;
  (void)a3;
  (void)a4;
  (void)a5;

  return fd_dup(current_task, (int)old_fd);
}

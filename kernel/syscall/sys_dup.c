#include "kernel/scheduler/scheduler.h"

long sys_dup(long old_fd, long a1, long a2, long a3, long a4, long a5) {
  (void)a1;
  (void)a2;
  (void)a3;
  (void)a4;
  (void)a5;

  if (old_fd < 0 || old_fd >= MAX_FDS) return -1;

  struct file* file = current_task->fd_table[old_fd];
  if (!file) return -1;

  return fd_install_ref(current_task, file);
}

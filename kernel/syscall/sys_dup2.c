#include "kernel/scheduler/scheduler.h"

long sys_dup2(long old_fd, long new_fd, long a2, long a3, long a4, long a5) {
  (void)a2;
  (void)a3;
  (void)a4;
  (void)a5;

  if (old_fd < 0 || old_fd >= MAX_FDS) return -1;
  if (new_fd < 0 || new_fd >= MAX_FDS) return -1;

  struct file* file = current_task->fd_table[old_fd];
  if (!file) return -1;

  if (old_fd == new_fd) return new_fd;

  // Acquire the new reference before dropping old target
  file_ref(file);

  struct file* replaced = current_task->fd_table[new_fd];
  current_task->fd_table[new_fd] = file;

  if (replaced) file_unref(replaced);

  return new_fd;
}

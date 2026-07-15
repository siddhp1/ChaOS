#include "fs/file.h"

#include "kernel/task.h"

int fd_alloc(struct task* task, struct file* file) {
  for (size_t i = 0; i < MAX_FDS; ++i) {
    if (task->fd_table[i] == NULL) {
      task->fd_table[i] = file;
      return 0;
    }
  }
  return -1;
}

struct file* fd_get(struct task* task, int fd) {
  if (fd < 0 || fd >= MAX_FDS) return NULL;
  return task->fd_table[fd];
}

int fd_close(struct task* task, int fd) {
  if (fd < 0 || fd >= MAX_FDS) return -1;

  struct file* file = task->fd_table[fd];
  if (!file) return -1;

  file->refcount--;
  task->fd_table[fd] = NULL;

  return 0;
}

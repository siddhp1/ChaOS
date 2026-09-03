#include "fs/file.h"

#include <stdint.h>

#include "kernel/task.h"
#include "mm/heap.h"

/**
 * @file
 * @brief Reference-counted open-file and descriptor-table implementation.
 */

struct file* file_alloc(const struct file_ops* ops, uint32_t flags,
                        void* data) {
  if (!ops) return NULL;

  struct file* file = (struct file*)kzalloc(sizeof(struct file));
  if (!file) return NULL;

  file->refcount = 1;
  // Position field is zeroed by allocator
  file->flags = flags;
  file->data = data;
  file->ops = ops;

  return file;
}

void file_ref(struct file* file) {
  if (!file) return;

  file->refcount++;
}

void file_unref(struct file* file) {
  if (!file) return;

  file->refcount--;
  if (file->refcount != 0) return;

  // Release underlying resource and free structure once refcount = 0
  if (file->ops && file->ops->release) {
    file->ops->release(file);
  }
  path_unref(&file->path);
  kfree(file);
}

int fd_install(struct task* task, struct file* file) {
  if (!task || !file) return -1;

  // Find first available index in the fd table
  for (int fd = 0; fd < MAX_FDS; ++fd) {
    if (task->fd_table[fd] == NULL) {
      task->fd_table[fd] = file;
      return fd;
    }
  }

  return -1;
}

int fd_install_at(struct task* task, struct file* file, int fd) {
  if (!task || !file || fd < 0 || fd >= MAX_FDS) return -1;

  if (task->fd_table[fd] != NULL) return -1;

  task->fd_table[fd] = file;

  return 0;
}

int fd_install_ref(struct task* task, struct file* file) {
  if (!task || !file) return -1;

  // Find first available index in the fd table
  for (int fd = 0; fd < MAX_FDS; ++fd) {
    if (task->fd_table[fd] == NULL) {
      file_ref(file);
      task->fd_table[fd] = file;
      return fd;
    }
  }

  return -1;
}

struct file* fd_get(struct task* task, int fd) {
  if (!task || fd < 0 || fd >= MAX_FDS) return NULL;
  return task->fd_table[fd];
}

int fd_close(struct task* task, int fd) {
  if (!task || fd < 0 || fd >= MAX_FDS) return -1;

  struct file* file = task->fd_table[fd];
  if (!file) return -1;

  task->fd_table[fd] = NULL;
  file_unref(file);

  return 0;
}

int fd_dup(struct task* task, int old_fd) {
  if (!task || old_fd < 0 || old_fd >= MAX_FDS) return -1;

  struct file* file = task->fd_table[old_fd];
  if (!file) return -1;

  return fd_install_ref(task, file);
}

void fd_table_close_all(struct task* task) {
  if (!task) return;

  for (int fd = 0; fd < MAX_FDS; ++fd) {
    if (task->fd_table[fd]) {
      struct file* file = task->fd_table[fd];

      task->fd_table[fd] = NULL;
      file_unref(file);
    }
  }
}

int fd_table_copy(struct task* dest, struct task* src) {
  if (!dest || !src) return -1;

  for (int fd = 0; fd < MAX_FDS; ++fd) {
    struct file* file = src->fd_table[fd];
    if (!file) continue;

    file_ref(file);
    dest->fd_table[fd] = file;  // Overwrites the destination table entry
  }

  return 0;
}

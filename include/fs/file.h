#ifndef FILE_H
#define FILE_H

#include <stddef.h>
#include <stdint.h>

#include "fs/vfs.h"

#define MAX_FDS 32

// Forward declarations
struct task;
struct file;

struct file_ops {
  int (*open)(struct inode* inode, struct file* file);
  long (*read)(struct file* file, void* buf, size_t len);
  long (*write)(struct file* file, const void* buf, size_t len);
  int (*release)(struct file* file);
};

struct file {
  uint32_t refcount;
  uint32_t flags;
  size_t position;
  struct path path;
  void* data;
  const struct file_ops* f_ops;
};

struct file* file_alloc(const struct file_ops* f_ops, uint32_t flags,
                        void* data);
void file_ref(struct file* file);
void file_unref(struct file* file);

// Transfers an existing owned reference into the fd table
int fd_install(struct task* task, struct file* file);
int fd_install_at(struct task* task, struct file* file, int fd);

// Creates and installs an additional reference
int fd_install_ref(struct task* task, struct file* file);

// Borrowed lookup
struct file* fd_get(struct task* task, int fd);

int fd_close(struct task* task, int fd);
int fd_dup(struct task* task, int old_fd);

void fd_table_close_all(struct task* task);
int fd_table_copy(struct task* dest, struct task* src);

#endif

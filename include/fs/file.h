#ifndef FILE_H
#define FILE_H

#include <stddef.h>
#include <stdint.h>

#define MAX_FDS 32

// Forward declaration
struct task;
struct file;

struct file_ops {
  long (*read)(struct file* file, void* buf, size_t len);
  long (*write)(struct file* file, const void* buf, size_t len);
  int (*close)(struct file* file);
};

struct file {
  uint32_t refcount;
  uint32_t flags;
  size_t position;
  void* data;
  const struct file_ops* ops;
};

// TODO: Implement fd helpers
// struct file* file_alloc(const struct file_ops* ops, void* data) {}
// void file_ref(struct file* file) {}
// void file_unref(struct file* file) {}

int fd_alloc(struct task* task, struct file* file);
struct file* fd_get(struct task* task, int fd);
int fd_close(struct task* task, int fd);

#endif

#ifndef INITRAMFS_H
#define INITRAMFS_H

#include <stddef.h>

#include "kernel/task.h"

struct initramfs_file {
  const char* name;
  void* data;
  size_t size;
};

void initramfs_init(void);
struct initramfs_file* initramfs_lookup(const char* filename);
struct task* load_init(void);

#endif

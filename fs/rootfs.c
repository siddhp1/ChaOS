#include <stddef.h>

#include "fs/vfs.h"

// Rootfs-specific node type
struct rootfs_node {
  const char* name;
  enum inode_type type;

  const void* data;
  size_t size;

  struct rootfs_node* children;
  size_t child_count;
};

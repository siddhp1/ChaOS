#include <stddef.h>

#include "fs/vfs.h"

// Rootfs data
static const char hello_data[] = "hello from vfs\n";

// Rootfs-specific node type
struct rootfs_node {
  const char* name;
  enum inode_type type;

  const void* data;
  size_t size;

  struct rootfs_node* children;
  size_t child_count;
};

static struct rootfs_node root_children[] = {
    {
        .name = "hello.txt",
        .type = INODE_REG,
        .data = hello_data,
        .size = sizeof(hello_data) - 1,
        .children = NULL,
        .child_count = 0,
    },
    {
        .name = "dev",
        .type = INODE_DIR,
        .data = NULL,
        .size = 0,
        .children = NULL,
        .child_count = 0,
    },
    {
        .name = "bin",
        .type = INODE_DIR,
        .data = NULL,
        .size = 0,
        .children = NULL,
        .child_count = 0,
    },
};

static struct rootfs_node root_node = {
    .name = "",
    .type = INODE_DIR,
    .data = NULL,
    .size = 0,
    .children = root_children,
    .child_count = sizeof(root_children) / sizeof(root_children[0]),
};

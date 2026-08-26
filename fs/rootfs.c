#include <stddef.h>

#include "fs/file.h"
#include "fs/vfs.h"
#include "kernel/string.h"

// Forward declaration
static int rootfs_lookup(struct inode* dir, const char* name,
                         struct dentry** out);
static long rootfs_read(struct file* file, void* buf, size_t count);

static const struct inode_ops rootfs_dir_inode_ops = {
    .lookup = rootfs_lookup,
};

static const struct file_ops rootfs_file_ops = {
    .read = rootfs_read,
    .write = NULL,
    .release = NULL,
};

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

static struct inode* rootfs_make_inode(struct rootfs_node* node) {
  if (!node) return NULL;

  struct inode* inode = inode_alloc();
  if (!inode) return NULL;

  inode->type = node->type;
  inode->size = node->size;
  inode->data = node;

  if (node->type == INODE_DIR) {
    inode->i_ops = &rootfs_dir_inode_ops;
    inode->f_ops = NULL;
  } else if (node->type == INODE_REG) {
    inode->i_ops = NULL;
    inode->f_ops = &rootfs_file_ops;
  } else {
    inode_unref(inode);
    return NULL;
  }

  return inode;
}

static struct dentry* rootfs_make_dentry(const char* name,
                                         struct dentry* parent,
                                         struct rootfs_node* node) {
  if (!name || !node) return NULL;

  struct dentry* dentry = dentry_alloc();
  if (!dentry) return NULL;

  strncpy(dentry->name, name, VFS_NAME_MAX);
  dentry->name[VFS_NAME_MAX] = '\0';
  dentry->parent = parent;
  dentry->inode = rootfs_make_inode(node);

  if (!dentry->inode) {
    dentry_unref(dentry);
    return NULL;
  }

  return dentry;
}

static int rootfs_lookup(struct inode* dir, const char* name,
                         struct dentry** out) {
  (void)dir;
  (void)name;
  (void)out;
  return -1;
}

static long rootfs_read(struct file* file, void* buf, size_t count) {
  (void)file;
  (void)buf;
  (void)count;
  return -1;
}

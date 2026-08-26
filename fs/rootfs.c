#include <stddef.h>

#include "fs/file.h"
#include "fs/vfs.h"
#include "kernel/errno.h"
#include "kernel/string.h"

// Forward declaration
static int rootfs_lookup(struct inode* dir, const char* name,
                         struct dentry** out);
static long rootfs_read(struct file* file, void* buf, size_t count);
static __attribute__((unused)) int rootfs_mount(struct mount* mount);

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
  if (!out) return -EINVAL;

  // Ensure caller doesn't receive a stale pointer on failure
  *out = NULL;

  if (!dir || !name) return -EINVAL;

  if (dir->type != INODE_DIR) return -ENOTDIR;

  struct rootfs_node* node = (struct rootfs_node*)dir->data;
  if (!node) return -EIO;

  for (size_t i = 0; i < node->child_count; ++i) {
    struct rootfs_node* child = &node->children[i];

    if (strcmp(child->name, name) == 0) {
      struct dentry* dentry = rootfs_make_dentry(name, NULL, child);
      if (!dentry) return -ENOMEM;

      *out = dentry;
      return 0;
    }
  }

  return -ENOENT;
}

static long rootfs_read(struct file* file, void* buf, size_t count) {
  if (!file || !buf) return -EINVAL;

  struct dentry* dentry = file->path.dentry;
  if (!dentry || !dentry->inode) return -EIO;

  struct inode* inode = dentry->inode;
  if (inode->type != INODE_REG) return -EISDIR;

  struct rootfs_node* node = (struct rootfs_node*)inode->data;
  if (!node || (!node->data && node->size != 0)) return -EIO;

  if (file->position >= node->size) return 0;

  size_t remaining = node->size - file->position;
  if (count > remaining) count = remaining;

  memcpy(buf, (const char*)node->data + file->position, count);
  file->position += count;

  return (long)count;
}

static __attribute__((unused)) int rootfs_mount(struct mount* mount) {
  if (!mount) return -EINVAL;

  struct superblock* sb = superblock_alloc();
  if (!sb) return -ENOMEM;

  struct dentry* root = rootfs_make_dentry("/", NULL, &root_node);
  if (!root) {
    superblock_unref(sb);
    return -ENOMEM;
  }

  root->parent = root;
  sb->root = root;
  mount->sb = sb;
  mount->root = root;

  return 0;
}

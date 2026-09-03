#include "fs/rootfs.h"

#include <stddef.h>

#include "fs/file.h"
#include "fs/vfs.h"
#include "kernel/errno.h"
#include "kernel/string.h"

/**
 * @file
 * @brief Built-in, read-only filesystem used as the VFS root.
 */

/**
 * @brief Looks up one child and returns a newly allocated dentry.
 * @param[in] dir Rootfs directory inode to search.
 * @param[in] name NUL-terminated child name.
 * @param[out] out Receives an owned dentry, or NULL on failure.
 * @return 0 on success, or `-EINVAL`, `-ENOTDIR`, `-EIO`, `-ENOMEM`, or
 * `-ENOENT`.
 */
static int rootfs_lookup(struct inode* dir, const char* name,
                         struct dentry** out);

/**
 * @brief Reads static node data from the open file's current position.
 * @param[in,out] file Open rootfs file whose position is advanced.
 * @param[out] buf Buffer receiving at most @p count bytes.
 * @param[in] count Maximum number of bytes to read.
 * @return Bytes read, zero at EOF, or `-EINVAL`, `-EIO`, or `-EISDIR`.
 */
static long rootfs_read(struct file* file, void* buf, size_t count);

/**
 * @brief Creates the rootfs superblock and root dentry for a mount.
 * @param[in,out] mount Newly allocated mount to populate.
 * @return 0 on success, `-EINVAL` for a NULL mount, or `-ENOMEM` on failure.
 */
static int rootfs_mount(struct mount* mount);

/** Directory operations shared by all rootfs directories. */
static const struct inode_ops rootfs_dir_inode_ops = {
    .lookup = rootfs_lookup,
};

/** Read-only file operations shared by all rootfs regular files. */
static const struct file_ops rootfs_file_ops = {
    .read = rootfs_read,
    .write = NULL,
    .release = NULL,
};

/** Contents of the built-in demonstration file. */
static const char hello_data[] = "hello from vfs\n";

/** Immutable backing node from which VFS inodes and dentries are created. */
struct rootfs_node {
  /** Borrowed, NUL-terminated entry name. */
  const char* name;
  /** Type assigned to generated inodes. */
  enum inode_type type;
  /** Borrowed regular-file contents, or NULL for a directory. */
  const void* data;
  /** Regular-file size in bytes. */
  size_t size;
  /** Borrowed array of directory children. */
  struct rootfs_node* children;
  /** Number of entries in the child array. */
  size_t child_count;
};

/** Children exposed directly beneath the root directory. */
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

/** Static backing node for `/`. */
static struct rootfs_node root_node = {
    .name = "",
    .type = INODE_DIR,
    .data = NULL,
    .size = 0,
    .children = root_children,
    .child_count = sizeof(root_children) / sizeof(root_children[0]),
};

/**
 * @brief Creates an inode backed by a static rootfs node.
 * @param[in] node Node that must outlive the inode.
 * @return Caller-owned inode, or NULL for invalid input or allocation failure.
 */
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

/**
 * @brief Creates a dentry and inode for a static rootfs node.
 * @param[in] name Name copied into the dentry.
 * @param[in] parent Parent retained by the new dentry; may be NULL.
 * @param[in] node Node that must outlive the generated inode.
 * @return Caller-owned dentry, or NULL on invalid input or allocation failure.
 */
static struct dentry* rootfs_make_dentry(const char* name,
                                         struct dentry* parent,
                                         struct rootfs_node* node) {
  if (!name || !node) return NULL;

  struct dentry* dentry = dentry_alloc();
  if (!dentry) return NULL;

  strncpy(dentry->name, name, VFS_NAME_MAX);
  dentry->name[VFS_NAME_MAX] = '\0';
  dentry_set_parent(dentry, parent);
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

static int rootfs_mount(struct mount* mount) {
  if (!mount) return -EINVAL;

  struct superblock* sb = superblock_alloc();
  if (!sb) return -ENOMEM;

  struct dentry* root = rootfs_make_dentry("/", NULL, &root_node);
  if (!root) {
    superblock_unref(sb);
    return -ENOMEM;
  }

  dentry_set_parent(root, root);
  sb->type = &rootfs_type;
  sb->root = root;
  mount->sb = sb;
  mount->root = root;

  return 0;
}

struct fs_type rootfs_type = {
    .name = "rootfs",
    .mount = rootfs_mount,
};

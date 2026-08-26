#ifndef VFS_H
#define VFS_H

#include <stddef.h>
#include <stdint.h>

#define VFS_NAME_MAX 255

// Forward declaration
struct dentry;
struct file_ops;
struct fs_type;
struct inode;

enum inode_type {
  INODE_REG,
  INODE_DIR,
};

struct inode_ops {
  int (*lookup)(struct inode* dir, const char* name, struct dentry** out);
};

struct inode {
  uint32_t refcount;
  enum inode_type type;
  size_t size;
  const struct inode_ops* i_ops;
  const struct file_ops* f_ops;
  void* data;
};

struct dentry {
  uint32_t refcount;
  char name[VFS_NAME_MAX + 1];
  struct dentry* parent;
  struct inode* inode;
};

struct superblock {
  uint32_t refcount;
  struct fs_type* type;
  struct dentry* root;
  void* data;
};

struct mount {
  uint32_t refcount;
  struct superblock* sb;
  struct dentry* root;
  void* data;
};

struct path {
  struct mount* mount;
  struct dentry* dentry;
};

struct fs_type {
  const char* name;
  int (*mount)(struct mount* mount);
};

struct inode* inode_alloc(void);
void inode_ref(struct inode* inode);
void inode_unref(struct inode* inode);

struct dentry* dentry_alloc(void);
void dentry_ref(struct dentry* dentry);
void dentry_unref(struct dentry* dentry);

struct superblock* superblock_alloc(void);
void superblock_ref(struct superblock* sb);
void superblock_unref(struct superblock* sb);

struct mount* mount_alloc(void);
void mount_ref(struct mount* mount);
void mount_unref(struct mount* mount);

int vfs_init(void);

#endif

#ifndef VFS_H
#define VFS_H

#include <stddef.h>
#include <stdint.h>

// TODO: Add or remove depending on usage
// Subset of VFS error codes
#define ENOENT 2
#define EIO 5
#define EBADF 9
#define ENOMEM 12
#define EACCES 13
#define EFAULT 14
#define ENOTDIR 20
#define EISDIR 21
#define EINVAL 22
#define ENOSYS 38
#define ENAMETOOLONG 36
#define EROFS 30

#define NAME_MAX 255

// TODO: Separate into different files
// ALL VFS-related stuff here

// Forward declarations
struct file_ops;
struct mount;

enum inode_type { INODE_DIR, INODE_REG, INODE_CHR };

struct inode {
  uint32_t refcount;
  enum inode_type type;
  size_t size;
  void* data;
  const struct inode_ops* i_ops;
  const struct file_ops* f_ops;
};

struct dentry {
  uint32_t refcount;
  char name[NAME_MAX + 1];
  struct dentry* parent;
  struct inode* inode;
  void* data;
};

struct fs_type {
  const char* name;
  int (*mount)(struct mount* mnt);
};

struct superblock {
  const struct fs_type* fs_type;
  struct dentry* root;
  void* data;
};

struct mount {
  struct superblock* sb;
  struct dentry* root;
};

struct path {
  struct mount* mnt;
  struct dentry* dentry;
};

struct inode_ops {
  int (*lookup)(struct inode* dir, const char* name, struct dentry** out);
};

#endif

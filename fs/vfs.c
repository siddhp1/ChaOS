#include "fs/vfs.h"

#include "fs/file.h"
#include "fs/rootfs.h"
#include "kernel/errno.h"
#include "kernel/string.h"
#include "mm/heap.h"

/**
 * @file
 * @brief Core VFS object lifetime, pathname lookup, and I/O dispatch.
 */

/** Globally mounted root; its initial reference is retained for kernel life. */
static struct mount* vfs_root_mount;

struct inode* inode_alloc(void) {
  struct inode* inode = (struct inode*)kzalloc(sizeof(struct inode));
  if (!inode) return NULL;

  inode->refcount = 1;
  return inode;
}

void inode_ref(struct inode* inode) {
  if (!inode) return;

  inode->refcount++;
}

void inode_unref(struct inode* inode) {
  if (!inode) return;

  inode->refcount--;
  if (inode->refcount != 0) return;

  kfree(inode);
}

struct dentry* dentry_alloc(void) {
  struct dentry* dentry = (struct dentry*)kzalloc(sizeof(struct dentry));
  if (!dentry) return NULL;

  dentry->refcount = 1;
  return dentry;
}

void dentry_ref(struct dentry* dentry) {
  if (!dentry) return;

  dentry->refcount++;
}

void dentry_unref(struct dentry* dentry) {
  if (!dentry) return;

  dentry->refcount--;
  if (dentry->refcount != 0) return;

  struct dentry* parent = dentry->parent;

  inode_unref(dentry->inode);
  kfree(dentry);

  if (parent && parent != dentry) dentry_unref(parent);
}

void dentry_set_parent(struct dentry* dentry, struct dentry* parent) {
  if (!dentry || dentry->parent == parent) return;

  struct dentry* old_parent = dentry->parent;
  if (parent && parent != dentry) dentry_ref(parent);

  dentry->parent = parent;

  if (old_parent && old_parent != dentry) dentry_unref(old_parent);
}

struct superblock* superblock_alloc(void) {
  struct superblock* sb =
      (struct superblock*)kzalloc(sizeof(struct superblock));
  if (!sb) return NULL;

  sb->refcount = 1;
  return sb;
}

void superblock_ref(struct superblock* sb) {
  if (!sb) return;

  sb->refcount++;
}

void superblock_unref(struct superblock* sb) {
  if (!sb) return;

  sb->refcount--;
  if (sb->refcount != 0) return;

  dentry_unref(sb->root);
  kfree(sb);
}

struct mount* mount_alloc(void) {
  struct mount* mount = (struct mount*)kzalloc(sizeof(struct mount));
  if (!mount) return NULL;

  mount->refcount = 1;
  return mount;
}

void mount_ref(struct mount* mount) {
  if (!mount) return;

  mount->refcount++;
}

void mount_unref(struct mount* mount) {
  if (!mount) return;

  mount->refcount--;
  if (mount->refcount != 0) return;

  superblock_unref(mount->sb);
  kfree(mount);
}

int vfs_init(void) {
  if (vfs_root_mount) return 0;

  struct mount* mount = mount_alloc();
  if (!mount) return -ENOMEM;

  int err = rootfs_type.mount(mount);
  if (err < 0) {
    mount_unref(mount);
    return err;
  }

  vfs_root_mount = mount;
  return 0;
}

void path_ref(struct path* path) {
  if (!path) return;

  mount_ref(path->mount);
  dentry_ref(path->dentry);
}

void path_unref(struct path* path) {
  if (!path) return;

  dentry_unref(path->dentry);
  mount_unref(path->mount);
  path->mount = NULL;
  path->dentry = NULL;
}

int vfs_lookup_path(const char* pathname, struct path* out) {
  if (!out) return -EINVAL;

  out->mount = NULL;
  out->dentry = NULL;

  if (!pathname || pathname[0] != '/') return -EINVAL;
  if (!vfs_root_mount || !vfs_root_mount->root) return -EIO;

  struct path current = {
      .mount = vfs_root_mount,
      .dentry = vfs_root_mount->root,
  };
  path_ref(&current);

  const char* cursor = pathname;

  while (*cursor != '\0') {
    while (*cursor == '/') ++cursor;
    if (*cursor == '\0') break;

    const char* component_start = cursor;
    while (*cursor != '\0' && *cursor != '/') ++cursor;

    size_t component_len = (size_t)(cursor - component_start);

    if (component_len > VFS_NAME_MAX) {
      path_unref(&current);
      return -ENAMETOOLONG;
    }

    struct inode* dir = current.dentry->inode;
    if (!dir || dir->type != INODE_DIR) {
      path_unref(&current);
      return -ENOTDIR;
    }

    if (component_len == 1 && component_start[0] == '.') continue;

    if (component_len == 2 && component_start[0] == '.' &&
        component_start[1] == '.') {
      struct dentry* parent = current.dentry->parent;

      if (parent && parent != current.dentry) {
        dentry_ref(parent);
        dentry_unref(current.dentry);
        current.dentry = parent;
      }

      continue;
    }

    if (!dir->i_ops || !dir->i_ops->lookup) {
      path_unref(&current);
      return -ENOTDIR;
    }

    char component[VFS_NAME_MAX + 1];
    memcpy(component, component_start, component_len);
    component[component_len] = '\0';

    struct dentry* next = NULL;
    int err = dir->i_ops->lookup(dir, component, &next);
    if (err < 0) {
      path_unref(&current);
      return err;
    }

    if (!next) {
      path_unref(&current);
      return -EIO;
    }

    dentry_set_parent(next, current.dentry);
    dentry_unref(current.dentry);
    current.dentry = next;
  }

  *out = current;
  return 0;
}
long vfs_read(struct file* file, void* buf, size_t count) {
  if (!file) return -EBADF;
  if (file->path.dentry && file->path.dentry->inode &&
      file->path.dentry->inode->type == INODE_DIR)
    return -EISDIR;
  if (!file->ops || !file->ops->read) return -EBADF;
  return file->ops->read(file, buf, count);
}

long vfs_write(struct file* file, const void* buf, size_t count) {
  if (!file) return -EBADF;
  if (file->path.dentry && file->path.dentry->inode &&
      file->path.dentry->inode->type == INODE_DIR)
    return -EISDIR;
  if (!file->ops || !file->ops->write) return -EBADF;
  return file->ops->write(file, buf, count);
}

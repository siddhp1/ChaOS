#include "fs/vfs.h"

#include "mm/heap.h"

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

  inode_unref(dentry->inode);
  kfree(dentry);
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

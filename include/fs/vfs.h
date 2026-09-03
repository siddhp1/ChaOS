#ifndef VFS_H
#define VFS_H

#include <stddef.h>
#include <stdint.h>

/**
 * @file
 * @brief Virtual filesystem objects, path lookup, and I/O dispatch.
 */

/** Maximum length of one pathname component, excluding its terminator. */
#define VFS_NAME_MAX 255

// Forward declaration
struct file;
struct dentry;
struct file_ops;
struct fs_type;
struct inode;

/** Type of object represented by an inode. */
enum inode_type {
  /** Regular file. */
  INODE_REG,
  /** Directory. */
  INODE_DIR,
};

/** Operations implemented by a filesystem for directory inodes. */
struct inode_ops {
  /**
   * @brief Looks up one child name in a directory.
   * @param[in] dir Directory inode to search.
   * @param[in] name NUL-terminated name of one component.
   * @param[out] out Receives an owned dentry reference on success and NULL on
   * failure.
   * @return 0 on success or a negative error.
   */
  int (*lookup)(struct inode* dir, const char* name, struct dentry** out);
};

/** Filesystem object metadata shared by dentries. */
struct inode {
  /** Number of owned references. */
  uint32_t refcount;
  /** Object type. */
  enum inode_type type;
  /** File size in bytes. */
  size_t size;
  /** Borrowed inode operations table, or NULL when unsupported. */
  const struct inode_ops* i_ops;
  /** Borrowed open-file operations table, or NULL when unsupported. */
  const struct file_ops* f_ops;
  /** Borrowed backend state; not released by the generic VFS. */
  void* data;
};

/** Named reference to an inode within the directory hierarchy. */
struct dentry {
  /** Number of owned references. */
  uint32_t refcount;
  /** NUL-terminated name of this entry. */
  char name[VFS_NAME_MAX + 1];
  /** Owned parent reference; the root may point to itself without a reference.
   */
  struct dentry* parent;
  /** Owned inode reference. */
  struct inode* inode;
};

/** State shared by one mounted filesystem instance. */
struct superblock {
  /** Number of owned references. */
  uint32_t refcount;
  /** Borrowed filesystem type descriptor. */
  struct fs_type* type;
  /** Owned root-dentry reference. */
  struct dentry* root;
  /** Borrowed backend state; not released by the generic VFS. */
  void* data;
};

/** Mounted filesystem tracked by the VFS. */
struct mount {
  /** Number of owned references. */
  uint32_t refcount;
  /** Owned superblock reference. */
  struct superblock* sb;
  /** Borrowed alias of the mounted root dentry. */
  struct dentry* root;
  /** Borrowed backend state; not released by the generic VFS. */
  void* data;
};

/** Resolved location containing owned mount and dentry references. */
struct path {
  /** Mount containing the resolved entry. */
  struct mount* mount;
  /** Resolved directory entry. */
  struct dentry* dentry;
};

/** Static descriptor for a filesystem implementation. */
struct fs_type {
  /** Borrowed, NUL-terminated filesystem name. */
  const char* name;
  /**
   * @brief Populates a newly allocated mount.
   * @param[in,out] mount Mount to populate with a superblock and root.
   * @return 0 on success or a negative error.
   */
  int (*mount)(struct mount* mount);
};

/**
 * @brief Allocates a zero-initialized inode with one owned reference.
 * @return Caller-owned inode, or NULL if allocation fails.
 * @see inode_unref()
 */
struct inode* inode_alloc(void);

/**
 * @brief Adds an owned inode reference.
 * @param[in,out] inode Inode to retain; NULL is a no-op.
 */
void inode_ref(struct inode* inode);

/**
 * @brief Drops an owned inode reference and frees it when the count reaches
 * zero.
 * @param[in,out] inode Inode to release; NULL is a no-op.
 */
void inode_unref(struct inode* inode);

/**
 * @brief Allocates a zero-initialized dentry with one owned reference.
 * @return Caller-owned dentry, or NULL if allocation fails.
 * @see dentry_unref()
 */
struct dentry* dentry_alloc(void);

/**
 * @brief Adds an owned dentry reference.
 * @param[in,out] dentry Dentry to retain; NULL is a no-op.
 */
void dentry_ref(struct dentry* dentry);

/**
 * @brief Drops a dentry reference and releases its inode and parent on final
 * use.
 * @param[in,out] dentry Dentry to release; NULL is a no-op.
 */
void dentry_unref(struct dentry* dentry);

/**
 * @brief Replaces a dentry's parent reference.
 *
 * A self-parent pointer is stored without adding a reference, allowing the root
 * dentry to represent `..` without a reference cycle.
 *
 * @param[in,out] dentry Dentry to update; NULL is a no-op.
 * @param[in] parent New parent, which may be NULL or @p dentry itself.
 */
void dentry_set_parent(struct dentry* dentry, struct dentry* parent);

/**
 * @brief Allocates a zero-initialized superblock with one owned reference.
 * @return Caller-owned superblock, or NULL if allocation fails.
 */
struct superblock* superblock_alloc(void);

/**
 * @brief Adds an owned superblock reference.
 * @param[in,out] sb Superblock to retain; NULL is a no-op.
 */
void superblock_ref(struct superblock* sb);

/**
 * @brief Drops a superblock reference and releases its root on final use.
 * @param[in,out] sb Superblock to release; NULL is a no-op.
 */
void superblock_unref(struct superblock* sb);

/**
 * @brief Allocates a zero-initialized mount with one owned reference.
 * @return Caller-owned mount, or NULL if allocation fails.
 */
struct mount* mount_alloc(void);

/**
 * @brief Adds an owned mount reference.
 * @param[in,out] mount Mount to retain; NULL is a no-op.
 */
void mount_ref(struct mount* mount);

/**
 * @brief Drops a mount reference and releases its superblock on final use.
 * @param[in,out] mount Mount to release; NULL is a no-op.
 */
void mount_unref(struct mount* mount);

/**
 * @brief Initializes the VFS and mounts rootfs at the global root.
 * @return 0 if initialized, or `-ENOMEM` or a rootfs mount error on failure.
 */
int vfs_init(void);

/**
 * @brief Resolves an absolute pathname from the global root.
 *
 * Repeated separators, `.` and `..` are accepted; `..` remains at the root.
 * On failure, @p out is cleared. On success, the caller owns its references and
 * must call path_unref().
 *
 * @param[in] pathname NUL-terminated absolute pathname.
 * @param[out] out Receives the resolved path.
 * @return 0 on success, or `-EINVAL`, `-EIO`, `-ENAMETOOLONG`, `-ENOTDIR`, or a
 * filesystem lookup error.
 */
int vfs_lookup_path(const char* pathname, struct path* out);

/**
 * @brief Dispatches a read through an open file's backend.
 * @param[in,out] file File to read; its position may change.
 * @param[out] buf Buffer receiving at most @p count bytes.
 * @param[in] count Maximum number of bytes to read.
 * @return Bytes read, zero at EOF, `-EBADF`, `-EISDIR`, or a backend error.
 */
long vfs_read(struct file* file, void* buf, size_t count);

/**
 * @brief Dispatches a write through an open file's backend.
 * @param[in,out] file File to write; its position may change.
 * @param[in] buf Buffer containing @p count bytes.
 * @param[in] count Number of bytes available to write.
 * @return Bytes written, `-EBADF`, `-EISDIR`, or a backend error.
 */
long vfs_write(struct file* file, const void* buf, size_t count);

/**
 * @brief Retains both non-NULL components of a path.
 * @param[in,out] path Path to retain; NULL is a no-op.
 * @see path_unref()
 */
void path_ref(struct path* path);

/**
 * @brief Releases both path components and clears their pointers.
 * @param[in,out] path Owned path to release; NULL is a no-op.
 * @see path_ref()
 */
void path_unref(struct path* path);

#endif

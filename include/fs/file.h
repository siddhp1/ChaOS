#ifndef FILE_H
#define FILE_H

#include <stddef.h>
#include <stdint.h>

#include "fs/vfs.h"

/**
 * @file
 * @brief Reference-counted open files and per-task descriptor tables.
 */

/** Maximum number of file descriptors in a task's descriptor table. */
#define MAX_FDS 32

// Forward declaration
struct task;
struct file;

/**
 * @brief Operations implemented by a file backend.
 *
 * The table must remain valid for the lifetime of every file that uses it. A
 * NULL operation means that it is unsupported. Read and write operations may
 * block according to the backend.
 */
struct file_ops {
  /**
   * @brief Reads up to @p len bytes from a file.
   * @param[in,out] file File being read; the operation may update its state.
   * @param[out] buf Buffer that receives the data.
   * @param[in] len Maximum number of bytes to read.
   * @return Number of bytes read, zero at end of input, or a negative error.
   */
  long (*read)(struct file* file, void* buf, size_t len);

  /**
   * @brief Writes up to @p len bytes to a file.
   * @param[in,out] file File being written; the operation may update its state.
   * @param[in] buf Buffer containing the data to write.
   * @param[in] len Number of bytes available to write.
   * @return Number of bytes written or a negative error.
   */
  long (*write)(struct file* file, const void* buf, size_t len);

  /**
   * @brief Releases backend state after the final reference is dropped.
   * @param[in,out] file File being released. The operation must not free it.
   * @return Backend-defined status; currently ignored by file_unref().
   */
  int (*release)(struct file* file);
};

/**
 * @brief Shared open-file state referenced by descriptor-table entries.
 *
 * A descriptor-table entry owns one reference. Multiple descriptors may share
 * the same file, including its position and flags. Reference counting is not
 * internally synchronized, so callers must serialize access. The operations
 * table must outlive the file.
 */
struct file {
  /** Number of owned references. */
  uint32_t refcount;
  /** Backend-defined open and status flags. */
  uint32_t flags;
  /** Current byte offset. */
  size_t position;
  /** Resolved VFS path owned by this file, or a null path for pathless files.
   */
  struct path path;
  /** Opaque state whose ownership and cleanup are defined by the backend. */
  void* data;
  /** Borrowed operations table that must outlive this file. */
  const struct file_ops* ops;
};

/**
 * @brief Allocates a zero-initialized file with one owned reference.
 * @param[in] ops Borrowed operations table that must outlive the file.
 * @param[in] flags Backend-defined flags stored in the file.
 * @param[in] data Opaque backend state; ownership remains backend-defined.
 * @return A caller-owned file with a null path and zero position, or NULL if
 * @p ops is NULL or allocation fails.
 * @see file_unref()
 */
struct file* file_alloc(const struct file_ops* ops, uint32_t flags, void* data);

/**
 * @brief Adds an owned reference to a file.
 * @param[in,out] file File to retain; NULL is a no-op.
 * @see file_unref()
 */
void file_ref(struct file* file);

/**
 * @brief Drops an owned reference to a file.
 *
 * The final reference invokes the backend's release operation, if present, and
 * then releases the associated VFS path and frees the file.
 *
 * @param[in,out] file File to release; NULL is a no-op.
 * @pre If non-NULL, the caller owns a reference to @p file.
 * @see file_ref()
 */
void file_unref(struct file* file);

/**
 * @brief Installs an owned file reference in the first free descriptor slot.
 *
 * Ownership transfers to the table on success and remains with the caller on
 * failure. Descriptor-table access is not internally synchronized.
 *
 * @param[in,out] task Task receiving the descriptor.
 * @param[in] file Owned file reference to install.
 * @return New descriptor, or -1 for invalid arguments or a full table.
 */
int fd_install(struct task* task, struct file* file);

/**
 * @brief Installs an owned file reference at a specified descriptor.
 *
 * Ownership transfers to the table on success and remains with the caller on
 * failure. Descriptor-table access is not internally synchronized.
 *
 * @param[in,out] task Task receiving the descriptor.
 * @param[in] file Owned file reference to install.
 * @param[in] fd Descriptor in the range `[0, MAX_FDS)`.
 * @return 0 on success, or -1 if an argument is invalid or the slot is
 * occupied.
 */
int fd_install_at(struct task* task, struct file* file, int fd);

/**
 * @brief Adds and installs a file reference in the first free slot.
 * @param[in,out] task Task receiving the descriptor.
 * @param[in,out] file File to retain and install.
 * @return New descriptor, or -1 for invalid arguments or a full table.
 */
int fd_install_ref(struct task* task, struct file* file);

/**
 * @brief Looks up a descriptor without adding a reference.
 * @param[in] task Task whose descriptor table is queried.
 * @param[in] fd Descriptor in the range `[0, MAX_FDS)`.
 * @return Borrowed file pointer that remains valid only while the entry is not
 * closed or replaced, or NULL if the descriptor is invalid or empty.
 */
struct file* fd_get(struct task* task, int fd);

/**
 * @brief Closes a descriptor and drops its owned file reference.
 * @param[in,out] task Task whose descriptor is closed.
 * @param[in] fd Descriptor in the range `[0, MAX_FDS)`.
 * @return 0 on success, or -1 if the descriptor is invalid or empty.
 */
int fd_close(struct task* task, int fd);

/**
 * @brief Duplicates a descriptor into the first free slot.
 * @param[in,out] task Task whose descriptor is duplicated.
 * @param[in] old_fd Descriptor to duplicate.
 * @return New descriptor, or -1 if @p old_fd is invalid or the table is full.
 */
int fd_dup(struct task* task, int old_fd);

/**
 * @brief Closes every descriptor owned by a task.
 * @param[in,out] task Task whose descriptor table is cleared; NULL is a no-op.
 */
void fd_table_close_all(struct task* task);

/**
 * @brief Copies a descriptor table and retains each installed file.
 *
 * Each copied descriptor owns a new reference to the same file as its source
 * descriptor.
 *
 * @param[in,out] dest Task receiving the copied descriptors.
 * @param[in] src Task whose descriptors are copied.
 * @return 0 on success, or -1 if either task is NULL.
 */
int fd_table_copy(struct task* dest, struct task* src);

#endif

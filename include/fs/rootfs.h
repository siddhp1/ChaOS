#ifndef ROOTFS_H
#define ROOTFS_H

#include "fs/vfs.h"

/**
 * @file
 * @brief Built-in read-only root filesystem type.
 */

/** Static rootfs descriptor used by vfs_init(). */
extern struct fs_type rootfs_type;

#endif

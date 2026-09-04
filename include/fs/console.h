#ifndef CONSOLE_H
#define CONSOLE_H

#include "fs/file.h"

/**
 * @file
 * @brief UART-backed console files and process standard-I/O setup.
 */

/**
 * @brief Reads edited input from the UART console.
 *
 * Input is echoed as it is read. Carriage return and newline are normalized to
 * `\n`; backspace removes the previous byte from the current read. The call
 * blocks until a line ending is received or @p len bytes have been accepted.
 *
 * @param[in] file Console file associated with the operation; currently
 * ignored and permitted to be NULL.
 * @param[out] buf Buffer that receives at most @p len bytes; may be NULL only
 * when @p len is zero.
 * @param[in] len Size of @p buf in bytes.
 * @return Number of bytes accepted, 0 when @p len is zero, or -1 if @p buf is
 * NULL for a nonzero-length read.
 */
long console_read(struct file* file, void* buf, size_t len);

/**
 * @brief Writes a buffer to the UART console, blocking as needed.
 *
 * The UART driver emits a carriage return before each newline.
 *
 * @param[in] file Console file associated with the operation; currently
 * ignored and permitted to be NULL.
 * @param[in] buf Buffer containing @p len bytes to write; may be NULL only when
 * @p len is zero.
 * @param[in] len Number of bytes to write.
 * @return Number of input bytes written, 0 when @p len is zero, or a negative
 * UART error (including -1 for a NULL buffer on a nonzero-length write).
 */
long console_write(struct file* file, const void* buf, size_t len);

/**
 * @brief Opens a reference-counted UART console file.
 * @param[in] flags Flags stored in the new file object.
 * @return Caller-owned, pathless file on success, or NULL if allocation fails.
 * @see file_unref()
 */
struct file* console_file_open(int flags);

/**
 * @brief Installs UART console files as standard input, output, and error.
 *
 * Descriptors 0, 1, and 2 must be empty. If any allocation or installation
 * fails, descriptors installed by this call are rolled back; other descriptors
 * are left unchanged.
 *
 * @param[in,out] task Task receiving the standard descriptors.
 * @return 0 on success, or -1 if @p task is NULL, a standard descriptor is
 * occupied, or setup fails.
 */
int process_stdio_init(struct task* task);

#endif

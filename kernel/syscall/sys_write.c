#include "drivers/uart.h"
#include "fs/file.h"
#include "kernel/scheduler/scheduler.h"
#include "kernel/user_access.h"
#include "syscall_handlers.h"

long sys_write(long fd, long buffer, long length, long a3, long a4, long a5) {
  struct file* file;
  if (fd < 0 || fd >= MAX_FDS) return -1;
  file = current_task->fd_table[fd];
  if (!file) return -1;

  if (length < 0) return -1;
  if (length == 0) return 0;

  const char* user_buffer = (const char*)buffer;
  long written = 0;

  // Kernel staging buffer to avoid direct EL1 reads from user pointer
  char kbuf[128];

  while (written < length) {
    long remaining = length - written;
    long chunk =
        (remaining > (long)sizeof(kbuf)) ? (long)sizeof(kbuf) : remaining;

    if (copy_from_user(kbuf, user_buffer + written, (uint64_t)chunk) < 0) {
      return -1;
    }

    long out = file->ops->write(file, kbuf, chunk);
    if (out < 0) {
      return (written > 0) ? written : -1;
    }

    written += out;
  }

  return written;
}

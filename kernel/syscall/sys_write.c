#include "drivers/uart.h"
#include "fs/file.h"
#include "kernel/scheduler/scheduler.h"
#include "kernel/user_access.h"
#include "syscall_handlers.h"

long sys_write(long fd, long buf, long len, long a3, long a4, long a5) {
  (void)a3;
  (void)a4;
  (void)a5;

  if (fd < 0 || fd >= MAX_FDS) return -1;

  if (len < 0) return -1;
  if (len == 0) return 0;

  struct file* file = fd_get(current_task, fd);
  if (!file) return -1;

  if (!file->ops || !file->ops->write) return -1;

  if (!user_range_ok((uintptr_t)buf, (uint64_t)len)) return -1;

  const char* user_buf = (const char*)buf;

  // Kernel staging buffer to avoid direct EL1 reads from user pointer
  char kbuf[128];

  long written = 0;

  while (written < len) {
    long remaining = len - written;
    long chunk =
        (remaining > (long)sizeof(kbuf)) ? (long)sizeof(kbuf) : remaining;

    if (copy_from_user(kbuf, user_buf + written, (uint64_t)chunk) < 0) {
      return written > 0 ? written : -1;
    }

    long result = file->ops->write(file, kbuf, chunk);

    if (result < 0) return (written > 0) ? written : result;
    if (result == 0) break;
    if (result > chunk) return written > 0 ? written : -1;

    written += result;
  }

  return written;
}

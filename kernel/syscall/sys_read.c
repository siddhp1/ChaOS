#include "drivers/uart.h"
#include "fs/file.h"
#include "kernel/scheduler/scheduler.h"
#include "kernel/user_access.h"

long sys_read(long fd, long buf, long len, long a3, long a4, long a5) {
  (void)a3;
  (void)a4;
  (void)a5;

  if (fd < 0 || fd >= MAX_FDS) return -1;

  if (len < 0) return -1;
  if (len == 0) return 0;

  struct file* file = fd_get(current_task, fd);
  if (!file) return -1;

  if (!file->ops || !file->ops->read) return -1;

  if (!user_range_ok((uintptr_t)buf, (uint64_t)len)) return -1;

  char* user_buf = (char*)buf;

  // Kernel staging buffer to avoid direct EL1 writes to user pointer
  char kbuf[128];

  long read = 0;

  while (read < len) {
    long long remaining = len - read;
    long chunk =
        remaining > (long)sizeof(kbuf) ? (long)sizeof(kbuf) : remaining;

    long result = file->ops->read(file, kbuf, chunk);

    if (result < 0) return read > 0 ? read : result;
    if (result == 0) break;
    if (result > chunk) return read > 0 ? read : -1;

    if (copy_to_user(user_buf + read, kbuf, (uint64_t)result) < 0) {
      return read > 0 ? read : -1;
    }

    read += result;

    if (result < chunk) break;
  }

  return read;
}

#include <stddef.h>
#include <stdint.h>

#include "fs/file.h"
#include "fs/vfs.h"
#include "kernel/scheduler/scheduler.h"
#include "kernel/user_access.h"

long sys_read(long fd, long buf, long len, long a3, long a4, long a5) {
  (void)a3;
  (void)a4;
  (void)a5;

  if (fd < 0 || fd >= MAX_FDS) return -1;
  if (len < 0) return -1;
  if (len == 0) return 0;

  struct file* file = fd_get(current_task, (int)fd);
  if (!file) return -1;

  if (!user_range_ok((uintptr_t)buf, (uint64_t)len)) return -1;

  // Perform only one read operation per syscall
  // Allow a short result when kernel buffer is smaller than requested length
  char kbuf[128];
  size_t chunk = (uint64_t)len > sizeof(kbuf) ? sizeof(kbuf) : (size_t)len;

  long result = vfs_read(file, kbuf, chunk);
  if (result < 0) return result;
  if ((size_t)result > chunk) return -1;

  if (result > 0 && copy_to_user((void*)buf, kbuf, (uint64_t)result) < 0) {
    return -1;
  }

  return result;
}

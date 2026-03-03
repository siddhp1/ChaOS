#include "libc.h"

long write(int fd, const void *buf, long len) {
  return syscall(SYS_WRITE, fd, (long)buf, len, 0, 0, 0);
}

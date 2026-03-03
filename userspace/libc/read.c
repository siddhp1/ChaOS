#include "libc.h"

long read(int fd, void *buf, long len) {
  return syscall(SYS_READ, fd, (long)buf, len, 0, 0, 0);
}

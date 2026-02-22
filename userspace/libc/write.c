#include "kernel/syscall.h"
#include "libc.h"

long write(int file_descriptor, const void* buffer, long length) {
  return syscall(SYS_WRITE, file_descriptor, (long)buffer, length, 0, 0, 0);
}

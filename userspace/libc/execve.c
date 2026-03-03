#include "libc.h"

int execve(const char *path, char *const argv[]) {
  (void)argv;  // argv not yet supported by kernel
  return (int)syscall(SYS_EXECVE, (long)path, 0, 0, 0, 0, 0);
}

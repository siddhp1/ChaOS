#include "libc.h"

long execve(const char* path, char* const argv[]) {
  (void)argv;  // argv not yet supported by kernel
  return syscall(SYS_EXECVE, (long)path, 0, 0, 0, 0, 0);
}

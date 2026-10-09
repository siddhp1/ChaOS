#include "libc.h"

long dup2(int old_fd, int new_fd) {
  return syscall(SYS_DUP2, (long)old_fd, (long)new_fd, 0, 0, 0, 0);
}

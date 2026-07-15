#include "libc.h"

void dup2(int old_fd, int new_fd) {
  syscall(SYS_DUP, (long)old_fd, (long)new_fd, 0, 0, 0, 0);
}

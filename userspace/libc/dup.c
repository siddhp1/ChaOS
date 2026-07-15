#include "libc.h"

void dup(int old_fd) { syscall(SYS_DUP, (long)old_fd, 0, 0, 0, 0, 0); }

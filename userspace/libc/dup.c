#include "libc.h"

long dup(int old_fd) { return syscall(SYS_DUP, (long)old_fd, 0, 0, 0, 0, 0); }

#include "libc.h"

long close(int fd) { return syscall(SYS_CLOSE, fd, 0, 0, 0, 0, 0); }

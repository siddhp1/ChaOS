#include "libc.h"

long fork(void) { return syscall(SYS_FORK, 0, 0, 0, 0, 0, 0); }

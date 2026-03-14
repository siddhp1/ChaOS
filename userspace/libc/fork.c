#include "libc.h"

int fork(void) { return (int)syscall(SYS_FORK, 0, 0, 0, 0, 0, 0); }

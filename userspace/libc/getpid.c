#include "libc.h"

int getpid(void) {
  return (int)syscall(SYS_GETPID, 0, 0, 0, 0, 0, 0);
}

#include "libc/libc.h"

long wait(int* status) {
  return syscall(SYS_WAIT, (long)status, 0, 0, 0, 0, 0);
}

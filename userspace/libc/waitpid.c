#include "libc/libc.h"

long waitpid(int pid, int* status) {
  return syscall(SYS_WAITPID, (long)pid, (long)status, 0, 0, 0, 0);
}

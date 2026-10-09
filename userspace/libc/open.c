#include "libc.h"

long open(const char* user_path_addr, int flags, int mode) {
  return syscall(SYS_OPEN, (long)user_path_addr, (long)flags, (long)mode, 0, 0,
                 0);
}

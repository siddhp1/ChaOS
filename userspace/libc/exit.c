#include "libc.h"

void exit(int status) {
  syscall(SYS_EXIT, status, 0, 0, 0, 0, 0);
  while (1);
}

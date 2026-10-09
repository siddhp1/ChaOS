#include "libc/libc.h"

int main(void) {
  write(1, "Hello, World!\n", 14);

  // Test file redirection syscalls
  int saved = dup(1);
  int other = open("/dev/console", 0, 0);

  dup2(other, 1);
  write(1, "test1\n", 5);
  write(4, "test4\n", 5);

  dup2(saved, 1);
  close(saved);
  close(other);

  return 0;
}

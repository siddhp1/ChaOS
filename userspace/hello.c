#include "libc/libc.h"

int main(void) {
  write(1, "Hello, World!\n", 14);

  exit(0);

  while (1);  // SHould not hit
  return 0;
}

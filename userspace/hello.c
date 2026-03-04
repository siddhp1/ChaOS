#include "libc/libc.h"

int main(void) {
  write(1, "Hello, World!\n", 14);
  return 0;
}

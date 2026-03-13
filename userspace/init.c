#include "libc/libc.h"

int main(void) {
  write(1, "=== OS Shell ===\n", 17);
  exit(0);
}

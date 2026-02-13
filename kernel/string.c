#include "kernel/string.h"

#include <stddef.h>

void* memset(void* s, int c, size_t n) {
  unsigned char* p = (unsigned char*)s;
  unsigned char v = (unsigned char)c;

  for (size_t i = 0; i < n; i++) {
    p[i] = v;
  }
  return s;
}

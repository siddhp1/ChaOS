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

void* memcpy(void* dst, const void* src, size_t n) {
  char* d = (char*)dst;
  const char* s = (const char*)src;

  for (size_t i = 0; i < n; i++) {
    d[i] = s[i];
  }
  return dst;
}

size_t strlen(const char* s) {
  size_t len = 0;
  while (*s++) len++;
  return len;
}

int strcmp(const char* a, const char* b) {
  while (*a && *a == *b) {
    a++;
    b++;
  }
  return (unsigned char)*a - (unsigned char)*b;
}

int strncmp(const char* a, const char* b, size_t n) {
  for (size_t i = 0; i < n; i++) {
    if (a[i] != b[i]) return (unsigned char)a[i] - (unsigned char)b[i];
    if (a[i] == '\0') return 0;
  }
  return 0;
}

char* strncpy(char* dst, const char* src, size_t n) {
  size_t i = 0;

  while (i < n && src[i] != '\0') {
    dst[i] = src[i];
    i++;
  }

  while (i < n) {
    dst[i] = '\0';
    i++;
  }

  return dst;
}

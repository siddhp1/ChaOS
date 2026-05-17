#ifndef ALIGN_H
#define ALIGN_H

#include <stddef.h>
#include <stdint.h>

static inline void* align_down(void* p, size_t num) {
  return (void*)((uintptr_t)p & ~(num - 1));
}

static inline void* align_up(void* p, size_t num) {
  return (void*)(((uintptr_t)p + num - 1) & ~(num - 1));
}

#endif

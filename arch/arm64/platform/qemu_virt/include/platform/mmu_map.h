#ifndef MMU_MAP_H
#define MMU_MAP_H

#include <stdbool.h>
#include <stdint.h>

#define MAP_SIZE 0x80000000UL

static inline bool phys_is_device(uintptr_t phys) {
  if (phys < 0x40000000) return true;
  return false;
}

#endif

#ifndef MMU_MAP_H
#define MMU_MAP_H

#include <stdbool.h>
#include <stdint.h>

#define MAP_SIZE (2UL * 1024 * 1024 * 1024)  // 2 GiB

static inline bool phys_is_device(uintptr_t phys) {
  if (phys < 0x40000000) return true;
  return false;
}

#endif

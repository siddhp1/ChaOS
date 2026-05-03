#ifndef MMU_MAP_H
#define MMU_MAP_H

#include <stdbool.h>
#include <stdint.h>

#define MAP_SIZE (1UL * 1024 * 1024 * 1024)  // 1 GiB

static inline bool phys_is_device(uintptr_t phys) {
  if (phys >= 0x3F000000) return true;
  return false;
}

#endif

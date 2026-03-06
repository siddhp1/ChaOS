#include "kernel/uaccess.h"

#include <stdint.h>

// Matches current EL0-mapped block
#define USER_VA_START 0x00400000ULL
#define USER_VA_END 0x00800000ULL

bool user_range_ok(uint64_t addr, uint64_t len) {
  if (len == 0) return true;
  if (addr < USER_VA_START) return false;
  if (addr >= USER_VA_END) return false;
  if (len > (USER_VA_END - addr)) return false;
  return true;
}

long copy_from_user(void* dst, const void* user_src, uint64_t len) {
  uint64_t src = (uint64_t)user_src;
  if (!user_range_ok(src, len)) {
    return -1;
  }

  // TODO: Switch to memcpy
  uint8_t* d = (uint8_t*)dst;
  const uint8_t* s = (const uint8_t*)user_src;

  for (uint64_t i = 0; i < len; i++) {
    d[i] = s[i];
  }

  return 0;
}

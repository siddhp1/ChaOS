#include "kernel/user_access.h"

#include <stdint.h>

#include "kernel/string.h"

#define USER_VA_START 0x00001000ULL
#define USER_VA_END 0x80000000ULL

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

  memcpy(dst, user_src, len);

  return 0;
}

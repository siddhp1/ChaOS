#include "kernel/user_access.h"

#include <stdbool.h>
#include <stdint.h>

#include "kernel/string.h"
#include "kernel/user_thread.h"

bool user_range_ok(uintptr_t addr, uint64_t len) {
  if (len == 0) return true;
  if (addr < USER_VIRT_START) return false;
  if (addr >= USER_VIRT_END) return false;
  if (len > (USER_VIRT_END - addr)) return false;
  return true;
}

long copy_from_user(void* dst, const void* src, uint64_t len) {
  if (!user_range_ok((uintptr_t)src, len)) {
    return -1;
  }

  // TODO: Add check for unmapped memory

  memcpy(dst, src, len);

  return 0;
}

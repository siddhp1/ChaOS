#ifndef USER_ACCESS_H
#define USER_ACCESS_H

#include <stdbool.h>
#include <stdint.h>

bool user_range_ok(uintptr_t addr, uint64_t len);
long copy_from_user(void* dst, const void* user_src, uint64_t len);

#endif

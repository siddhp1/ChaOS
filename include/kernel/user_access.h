#ifndef USER_ACCESS_H
#define USER_ACCESS_H

#include <stdint.h>

long copy_from_user(void* dst, const void* user_src, uint64_t len);

#endif

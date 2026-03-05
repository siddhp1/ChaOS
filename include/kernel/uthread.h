#ifndef UTHREAD_H
#define UTHREAD_H

#include <stdint.h>

#include "task.h"

// TODO: Consider returning PID
int32_t uthread_create(uint64_t user_entry, uint64_t user_sp, uint64_t ttbr0);

#endif

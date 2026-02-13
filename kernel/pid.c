#include "pid.h"

#include <stdint.h>

static int32_t next_pid = 1;

int32_t pid_alloc(void) { return next_pid++; }

#include "kernel/context.h"

#include <stddef.h>
#include <stdint.h>

#include "kernel/pstate.h"

void context_init(context_t* ctx, uintptr_t entry_point, uintptr_t stack_top) {
  uint8_t* p = (uint8_t*)ctx;
  for (size_t i = 0; i < sizeof(context_t); i++) {
    p[i] = 0;
  }
  ctx->pc = entry_point;
  ctx->sp = stack_top;
  ctx->pstate = PSTATE_KERNEL_INIT;
}

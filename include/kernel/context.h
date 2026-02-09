#ifndef CONTEXT_H
#define CONTEXT_H

#include <stdint.h>

typedef struct {
  uint64_t r0;
  uint64_t r1;
  uint64_t r2;
  uint64_t r3;
  uint64_t r4;
  uint64_t r5;
  uint64_t r6;
  uint64_t r7;
  uint64_t r8;
  uint64_t r9;
  uint64_t r10;
  uint64_t r11;
  uint64_t pstate;
  uintptr_t sp;
  uintptr_t lr;
  uintptr_t pc;
} context_t;

void context_init(context_t* ctx, uintptr_t entry_point, uintptr_t stack_top);

#endif

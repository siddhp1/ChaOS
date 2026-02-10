#ifndef CPU_CONTEXT_H
#define CPU_CONTEXT_H

#include <stdint.h>

struct cpu_context {
  uint64_t x19;
  uint64_t x20;
  uint64_t x21;
  uint64_t x22;
  uint64_t x23;
  uint64_t x24;
  uint64_t x25;
  uint64_t x26;
  uint64_t x27;
  uint64_t x28;
  uint64_t fp;  // x29
  uint64_t lr;  // x30
  uint64_t sp;
};

#endif

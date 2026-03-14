#ifndef TRAP_H
#define TRAP_H

#include <stdint.h>

struct trapframe {
  uint64_t x[31];
  uint64_t _pad_x30;
  uint64_t elr_el1;
  uint64_t spsr_el1;
  uint64_t sp_el0;
  uint64_t _pad_sp;
};

#endif

#ifndef TRAP_H
#define TRAP_H

#include <stdint.h>

// TODO: add sp_el0
struct trapframe{
  uint64_t x[31];
  uint64_t _pad;
  uint64_t elr_el1;
  uint64_t spsr_el1;
};

#endif

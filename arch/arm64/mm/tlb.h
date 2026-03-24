#ifndef TLB_H
#define TLB_H

#include <stdint.h>

// TODO: Add ASID
static inline void tlb_flush_addr(uint64_t va) {
  asm volatile("dsb ishst" ::: "memory");
  asm volatile("tlbi vaae1is, %0" ::"r"(va >> 12) : "memory");
  asm volatile("dsb ish" ::: "memory");
  asm volatile("isb" ::: "memory");
}

static inline void tlb_flush_all(void) {
  asm volatile("tlbi vmalle1is" ::: "memory");
  asm volatile("dsb ish" ::: "memory");
  asm volatile("isb" ::: "memory");
}

#endif

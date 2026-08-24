#include "mm/tlb.h"

#include <stdint.h>

// TODO: Add ASID
void tlb_flush_addr(uint64_t va) {
  __asm__ volatile("dsb ishst" ::: "memory");
  __asm__ volatile("tlbi vaae1is, %0" ::"r"(va >> 12) : "memory");
  __asm__ volatile("dsb ish" ::: "memory");
  __asm__ volatile("isb" ::: "memory");
}

void tlb_flush_all(void) {
  __asm__ volatile("tlbi vmalle1is" ::: "memory");
  __asm__ volatile("dsb ish" ::: "memory");
  __asm__ volatile("isb" ::: "memory");
}

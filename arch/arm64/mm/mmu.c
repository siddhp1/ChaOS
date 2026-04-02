#include "mm/mmu.h"

#include <stdint.h>

#include "mm/pgtable.h"
#include "mm/tlb.h"

#define MAIR_ATTR_DEVICE 0x00
#define MAIR_ATTR_NORMAL 0xFF
#define MAIR_VALUE ((MAIR_ATTR_DEVICE << 0) | (MAIR_ATTR_NORMAL << 8))

#define TCR_T0SZ 16   // 48-bit virtual addresses for TTBR0
#define TCR_T1SZ 16   // 48-bit virtual addresses for TTBR1
#define TCR_TG0 0b00  // 4 KiB granule size
#define TCR_TG1 0b10  // 4 KiB granule size
#define TCR_VALUE \
  ((TCR_T0SZ << 0) | (TCR_T1SZ << 16) | (TCR_TG0 << 14) | (TCR_TG1 << 30))

void enable_mmu(uintptr_t ttbr0, uintptr_t ttbr1) {
  asm volatile("msr MAIR_EL1, %0" ::"r"(MAIR_VALUE));
  asm volatile("msr TCR_EL1, %0" ::"r"(TCR_VALUE));
  asm volatile("msr TTBR0_EL1, %0" ::"r"(ttbr0));
  asm volatile("msr TTBR1_EL1, %0" ::"r"(ttbr1));

  asm volatile("dsb ishst" ::: "memory");
  asm volatile("isb");

  tlb_flush_all();

  uint64_t sctlr;
  asm volatile("mrs %0, SCTLR_EL1" : "=r"(sctlr));
  // 0: MMU enable; 2: data cache enable; 12: instruction cache enable
  sctlr |= (1 << 0) | (1 << 2) | (1 << 12);
  asm volatile("msr SCTLR_EL1, %0" ::"r"(sctlr));

  asm volatile("dsb ishst" ::: "memory");
  asm volatile("isb");
}

void mmu_init(void) {
  uintptr_t l0_identity_table = setup_identity_tables();
  uintptr_t l0_higher_half_table = setup_higher_half_tables();
  enable_mmu(l0_identity_table, l0_higher_half_table);
}

void set_ttbr0(uintptr_t phys) {
  asm volatile("msr TTBR0_EL1, %0" ::"r"(phys) : "memory");
  tlb_flush_all();
}

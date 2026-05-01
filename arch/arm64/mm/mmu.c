#include "mm/mmu.h"

#include <stdint.h>

#include "mm/pgtable.h"
#include "mm/tlb.h"

#define MAIR_ATTR_DEVICE (0x00 << 0)
#define MAIR_ATTR_NORMAL (0xFF << 8)
#define MAIR_VALUE (MAIR_ATTR_DEVICE | MAIR_ATTR_NORMAL)

// 48-bit virtual addresses
#define TCR_T0SZ (16 << 0)
#define TCR_T1SZ (16 << 16)

// 4 KiB granule size
#define TCR_TG0 (0b00 << 14)
#define TCR_TG1 (0b10 << 30)

#define TCR_RGN_WBWA 0b01  // Write-back, write-allocate
#define TCR_SH_INNER 0b11  // Inner shareable

// Inner cacheability
#define TCR_IRGN0 (TCR_RGN_WBWA << 8)
#define TCR_IRGN1 (TCR_RGN_WBWA << 24)

// Outer cacheability
#define TCR_ORGN0 (TCR_RGN_WBWA << 10)
#define TCR_ORGN1 (TCR_RGN_WBWA << 26)

// Shareability
#define TCR_SH0 (TCR_SH_INNER << 12)
#define TCR_SH1 (TCR_SH_INNER << 28)

#define TCR_VALUE                                                             \
  TCR_T0SZ | TCR_T1SZ | TCR_TG0 | TCR_TG1 | TCR_IRGN0 | TCR_ORGN0 | TCR_SH0 | \
      TCR_IRGN1 | TCR_ORGN1 | TCR_SH1

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

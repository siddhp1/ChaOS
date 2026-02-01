#include "asm/mmu.h"

#include <stdint.h>

#include "asm/pgtable.h"
#include "asm/tlb.h"

static uint64_t l0_table[1024] __attribute__((aligned(0x1000)));
static uint64_t l1_table[1024] __attribute__((aligned(0x1000)));

void setup_page_tables(void) {
  for (int i = 0; i < 1024; i++) {
    l0_table[i] = 0;
    l1_table[i] = 0;
  }

  for (int i = 0; i < NUM_KERNEL_BLOCKS; i++) {
    uintptr_t phys = (uintptr_t)(i * PAGE_SIZE_2MB);
    l1_table[i] = (phys & 0xFFFFFFFFFFE00000ULL) | PTE_VALID | PTE_AF |
                  PTE_SH_INNER | PTE_ATTRINDX(0) |
                  0;  // UXN cleared for kernel code
  }

  uintptr_t l1_base = (uintptr_t)l1_table;
  l0_table[0] = (l1_base & 0x0000FFFFFFFFF000ULL) | PTE_VALID | PTE_TABLE;

  asm volatile("dsb ishst" ::: "memory");
  asm volatile("isb");

  tlb_flush_all();

  uintptr_t l0_base = (uintptr_t)l0_table;
  asm volatile("msr TTBR1_EL1, %0" ::"r"(l0_base));
}

static inline void write_mair_el1(uint64_t val) {
  asm volatile("msr MAIR_EL1, %0" ::"r"(val));
}

static inline void write_tcr_el1(uint64_t val) {
  asm volatile("msr TCR_EL1, %0" ::"r"(val));
}

static inline void write_ttbr0_el1(uintptr_t val) {
  asm volatile("msr TTBR0_EL1, %0" ::"r"(val));
}

static inline void write_ttbr1_el1(uintptr_t val) {
  asm volatile("msr TTBR1_EL1, %0" ::"r"(val));
}

void enable_mmu(uintptr_t ttbr0, uintptr_t ttbr1) {
  write_mair_el1(MAIR_VALUE);
  write_tcr_el1(TCR_VALUE);

  write_ttbr0_el1(ttbr0);
  write_ttbr1_el1(ttbr1);

  asm volatile("dsb ishst" ::: "memory");
  asm volatile("isb");
  tlb_flush_all();

  uint64_t sctlr;
  asm volatile("mrs %0, SCTLR_EL1" : "=r"(sctlr));
  sctlr |= (1 << 0) | (1 << 2) | (1 << 12);
  asm volatile("msr SCTLR_EL1, %0" ::"r"(sctlr));
  asm volatile("isb");
}

void mmu_init(void) {
  setup_page_tables();
  enable_mmu((uintptr_t)l0_table, (uintptr_t)l0_table);
}

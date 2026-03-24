#include "mm/mmu.h"

#include <stdint.h>

#include "mm/page.h"
#include "mm/pgtable.h"
#include "tlb.h"

#define DEVICE_PHYS_BASE 0x00000000UL
#define KERNEL_PHYS_BASE 0x40000000UL

#define MAIR_ATTR_DEVICE 0x00
#define MAIR_ATTR_NORMAL 0xFF
#define MAIR_VALUE ((MAIR_ATTR_DEVICE << 0) | (MAIR_ATTR_NORMAL << 8))

#define TCR_T0SZ 16   // 48-bit virtual addresses for TTBR0
#define TCR_T1SZ 16   // 48-bit virtual addresses for TTBR1
#define TCR_TG0 0b00  // 4 KiB granule size
#define TCR_TG1 0b10  // 4 KiB granule size
#define TCR_VALUE \
  ((TCR_T0SZ << 0) | (TCR_T1SZ << 16) | (TCR_TG0 << 14) | (TCR_TG1 << 30))

#define NLTA_MASK 0x0000FFFFFFFFF000ULL
#define L2_OAB_MASK 0x0000FFFFFFE00000ULL

#define NUM_TABLE_ENTRIES 512

#define L2_BLOCK_SIZE (1UL << 21)  // 2 MiB

// Identity mapping (TTBR0)
static uint64_t l0_table[NUM_TABLE_ENTRIES] __attribute__((aligned(PAGE_SIZE)));
static uint64_t l1_table[NUM_TABLE_ENTRIES] __attribute__((aligned(PAGE_SIZE)));

// Higher-half mapping (TTBR1)
static uint64_t l0_table_hi[NUM_TABLE_ENTRIES]
    __attribute__((aligned(PAGE_SIZE)));
static uint64_t l1_table_hi[NUM_TABLE_ENTRIES]
    __attribute__((aligned(PAGE_SIZE)));
static uint64_t l2_table_device[NUM_TABLE_ENTRIES]
    __attribute__((aligned(PAGE_SIZE)));

uint64_t* get_kernel_l0_table(void) { return l0_table_hi; }

void setup_page_tables(void) {
  for (int i = 0; i < NUM_TABLE_ENTRIES; i++) {
    l0_table[i] = 0;
    l1_table[i] = 0;
    l0_table_hi[i] = 0;
    l1_table_hi[i] = 0;
    l2_table_device[i] = 0;
  }

  l1_table[0] =
      (DEVICE_PHYS_BASE) | PTE_VALID | PTE_AF | PTE_SH_INNER | PTE_ATTRINDX(0);
  l1_table[1] =
      (KERNEL_PHYS_BASE) | PTE_VALID | PTE_AF | PTE_SH_INNER | PTE_ATTRINDX(1);

  uintptr_t l1_base = (uintptr_t)l1_table;
  l0_table[0] = (l1_base & NLTA_MASK) | PTE_VALID | PTE_TABLE;

  for (int i = 0; i < NUM_TABLE_ENTRIES; i++) {
    uintptr_t phys = (uintptr_t)(i * L2_BLOCK_SIZE);
    l2_table_device[i] = (phys & L2_OAB_MASK) | PTE_VALID | PTE_AF |
                         PTE_SH_INNER | PTE_ATTRINDX(0) | PTE_UXN | PTE_PXN;
  }
  uintptr_t l2_device_base = (uintptr_t)l2_table_device;
  l1_table_hi[0] = (l2_device_base & NLTA_MASK) | PTE_VALID | PTE_TABLE;
  l1_table_hi[1] =
      (KERNEL_PHYS_BASE) | PTE_VALID | PTE_AF | PTE_SH_INNER | PTE_ATTRINDX(1);

  uintptr_t l1_hi_base = (uintptr_t)l1_table_hi;
  l0_table_hi[0] = (l1_hi_base & NLTA_MASK) | PTE_VALID | PTE_TABLE;
}

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
  setup_page_tables();
  enable_mmu((uintptr_t)l0_table, (uintptr_t)l0_table_hi);
}

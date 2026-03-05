#include "mm/mmu.h"

#include <stdint.h>

#include "kernel/printk.h"
#include "kernel/scheduler.h"
#include "mm/kmap.h"
#include "mm/page.h"
#include "pgtable.h"
#include "tlb.h"

// Identity mapping (TTBR0)
static uint64_t l0_table[512] __attribute__((aligned(0x1000)));
static uint64_t l1_table[512] __attribute__((aligned(0x1000)));

// Higher-half mapping (TTBR1)
static uint64_t l0_table_hi[512] __attribute__((aligned(0x1000)));
static uint64_t l1_table_hi[512] __attribute__((aligned(0x1000)));
static uint64_t l2_table_device[512] __attribute__((aligned(0x1000)));

static uintptr_t kernel_ttbr0_root;

static inline void zero_table(uint64_t* table) {
  for (int i = 0; i < PTRS_PER_TABLE; i++) {
    table[i] = 0;
  }
}

static inline uint64_t make_table_desc(uintptr_t phys) {
  return (phys & 0x0000FFFFFFFFF000ULL) | PTE_VALID | PTE_TABLE;
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

uintptr_t mmu_kernel_ttbr0(void) { return kernel_ttbr0_root; }

uintptr_t mmu_create_user_ttbr0(void) {
  struct page* l0_page = alloc_page();
  if (!l0_page) return 0;

  struct page* l1_page = alloc_page();
  if (!l1_page) {
    free_page(l0_page);
    return 0;
  }

  uint64_t* l0 = (uint64_t*)kmap(l0_page);
  uint64_t* l1 = (uint64_t*)kmap(l1_page);

  zero_table(l0);
  zero_table(l1);

  // l1[0] = (0x00000000ULL) | PTE_VALID | PTE_AF | PTE_SH_INNER |
  //         PTE_ATTRINDX(0) | PTE_AP_EL1_RW | PTE_UXN | PTE_PXN;

  l1[1] = (0x40000000ULL) | PTE_VALID | PTE_AF | PTE_SH_INNER |
          PTE_ATTRINDX(1) | PTE_AP_EL0_RW;

  l0[0] = make_table_desc(page_to_phys(l1_page));

  return page_to_phys(l0_page);
}

uintptr_t mmu_current_user_ttbr0(void) {
  if (!current_task || !current_task->ttbr0) {
    return kernel_ttbr0_root;
  }
  return current_task->ttbr0;
}

void mmu_switch_ttbr0(uintptr_t ttbr0) {
  uintptr_t root = ttbr0 ? ttbr0 : kernel_ttbr0_root;

  root &= 0x0000FFFFFFFFF000ULL;
  if (!root) {
    printk("mmu_switch_ttbr0: invalid root\n");
    return;
  }

  write_ttbr0_el1(root);
  asm volatile("isb" ::: "memory");

  // asm volatile("dsb ishst" ::: "memory");
  // asm volatile("isb");
  // tlb_flush_all();
}

void setup_page_tables(void) {
  for (int i = 0; i < 512; i++) {
    l0_table[i] = 0;
    l1_table[i] = 0;
    l0_table_hi[i] = 0;
    l1_table_hi[i] = 0;
    l2_table_device[i] = 0;
  }

  l1_table[0] =
      (0x00000000ULL) | PTE_VALID | PTE_AF | PTE_SH_INNER | PTE_ATTRINDX(0);
  l1_table[1] =
      (0x40000000ULL) | PTE_VALID | PTE_AF | PTE_SH_INNER | PTE_ATTRINDX(1);

  uintptr_t l1_base = (uintptr_t)l1_table;
  l0_table[0] = (l1_base & 0x0000FFFFFFFFF000ULL) | PTE_VALID | PTE_TABLE;

  for (int i = 0; i < 512; i++) {
    uintptr_t phys = (uintptr_t)(i * L2_BLOCK_SIZE);
    l2_table_device[i] = (phys & 0x0000FFFFFFE00000ULL) | PTE_VALID | PTE_AF |
                         PTE_SH_INNER | PTE_ATTRINDX(0) | PTE_UXN | PTE_PXN;
  }
  uintptr_t l2_device_base = (uintptr_t)l2_table_device;
  l1_table_hi[0] =
      (l2_device_base & 0x0000FFFFFFFFF000ULL) | PTE_VALID | PTE_TABLE;
  l1_table_hi[1] =
      (PHYS_KERNEL_BASE) | PTE_VALID | PTE_AF | PTE_SH_INNER | PTE_ATTRINDX(1);

  uintptr_t l1_hi_base = (uintptr_t)l1_table_hi;
  l0_table_hi[0] = (l1_hi_base & 0x0000FFFFFFFFF000ULL) | PTE_VALID | PTE_TABLE;

  asm volatile("dsb ishst" ::: "memory");
  asm volatile("isb");

  tlb_flush_all();
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
  kernel_ttbr0_root = (uintptr_t)l0_table;
  enable_mmu((uintptr_t)l0_table, (uintptr_t)l0_table_hi);
}

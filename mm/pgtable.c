#include "mm/pgtable.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "kernel/printk.h"
#include "kernel/string.h"
#include "mm/kmap.h"
#include "mm/page.h"
#include "mm/tlb.h"
#include "mm/user_pgtable.h"
#include "platform/mmu_map.h"

#define L0_INDEX(va) (((va) >> 39) & 0x1FF)
#define L1_INDEX(va) (((va) >> 30) & 0x1FF)
#define L2_INDEX(va) (((va) >> 21) & 0x1FF)
#define L3_INDEX(va) (((va) >> 12) & 0x1FF)

#define PTE_L2_OAB_MASK 0x0000FFFFFFE00000ULL
#define PTE_L2_OAB(pte) ((pte) & PTE_L2_OAB_MASK)

#define L1_BLOCK_SIZE (1UL << 30)  // 1 GiB
#define L2_BLOCK_SIZE (1UL << 21)  // 2 MiB

#define NUM_L1_PTES ((MAP_SIZE + L1_BLOCK_SIZE - 1) / L1_BLOCK_SIZE)

static uint64_t l0_identity_table[NUM_TABLE_ENTRIES]
    __attribute__((aligned(PAGE_SIZE)));
static uint64_t l1_identity_table[NUM_TABLE_ENTRIES]
    __attribute__((aligned(PAGE_SIZE)));
static uint64_t l2_identity_tables[NUM_L1_PTES][NUM_TABLE_ENTRIES]
    __attribute__((aligned(PAGE_SIZE)));

static uint64_t l0_higher_half_table[NUM_TABLE_ENTRIES]
    __attribute__((aligned(PAGE_SIZE)));
static uint64_t l1_higher_half_table[NUM_TABLE_ENTRIES]
    __attribute__((aligned(PAGE_SIZE)));
static uint64_t l2_higher_half_tables[NUM_L1_PTES][NUM_TABLE_ENTRIES]
    __attribute__((aligned(PAGE_SIZE)));

static uint64_t* walk_to_l3(uint64_t* l0_table, uint64_t va, bool create) {
  uint64_t l0_idx = L0_INDEX(va);
  uint64_t l0_entry = l0_table[l0_idx];

  uint64_t* l1_table;
  if (!PTE_IS_VALID(l0_entry)) {
    if (!create) return NULL;

    uint64_t l1_phys = alloc_page_table();
    if (!l1_phys) return NULL;

    l0_table[l0_idx] = PTE_NLTA(l1_phys) | PTE_VALID | PTE_TABLE;
    l1_table = (uint64_t*)(KERNEL_VIRT_BASE + l1_phys);
  } else if (PTE_IS_TABLE(l0_entry)) {
    l1_table = (uint64_t*)(KERNEL_VIRT_BASE + PTE_NLTA(l0_entry));
  } else {
    return NULL;
  }

  uint64_t l1_idx = L1_INDEX(va);
  uint64_t l1_entry = l1_table[l1_idx];

  uint64_t* l2_table;
  if (!PTE_IS_VALID(l1_entry)) {
    if (!create) return NULL;

    uint64_t l2_phys = alloc_page_table();
    if (!l2_phys) return NULL;

    l1_table[l1_idx] = PTE_NLTA(l2_phys) | PTE_VALID | PTE_TABLE;
    l2_table = (uint64_t*)(KERNEL_VIRT_BASE + l2_phys);
  } else if (PTE_IS_TABLE(l1_entry)) {
    l2_table = (uint64_t*)(KERNEL_VIRT_BASE + PTE_NLTA(l1_entry));
  } else {
    return NULL;
  }

  uint64_t l2_idx = L2_INDEX(va);
  uint64_t l2_entry = l2_table[l2_idx];

  if (!PTE_IS_VALID(l2_entry)) {
    if (!create) return NULL;

    uint64_t l3_phys = alloc_page_table();
    if (!l3_phys) return NULL;

    l2_table[l2_idx] = PTE_L3_OAB(l3_phys) | PTE_VALID | PTE_TABLE;
    return (uint64_t*)(KERNEL_VIRT_BASE + l3_phys);
  } else if (PTE_IS_TABLE(l2_entry)) {
    return (uint64_t*)(KERNEL_VIRT_BASE + PTE_NLTA(l2_entry));
  }

  return NULL;
}

uint64_t* get_kernel_l0_table(void) { return l0_higher_half_table; }

uintptr_t setup_identity_tables(void) {
  memset(l0_identity_table, 0, sizeof(l0_identity_table));
  memset(l1_identity_table, 0, sizeof(l1_identity_table));
  memset(l2_identity_tables, 0, sizeof(l2_identity_tables));

  l0_identity_table[0] =
      PTE_NLTA((uintptr_t)l1_identity_table) | PTE_TABLE | PTE_VALID;

  for (size_t l1_idx = 0; l1_idx < NUM_L1_PTES; l1_idx++) {
    l1_identity_table[l1_idx] =
        PTE_NLTA((uintptr_t)l2_identity_tables[l1_idx]) | PTE_TABLE | PTE_VALID;

    for (size_t l2_idx = 0; l2_idx < NUM_TABLE_ENTRIES; l2_idx++) {
      uintptr_t phys = (l1_idx * L1_BLOCK_SIZE) + (l2_idx * L2_BLOCK_SIZE);

      uint64_t attrs = PTE_AF | PTE_VALID;
      if (phys_is_device(phys)) {
        attrs |= PTE_ATTRINDX(0) | PTE_UXN | PTE_PXN;
      } else {
        attrs |= PTE_ATTRINDX(1) | PTE_SH_INNER;
      }

      l2_identity_tables[l1_idx][l2_idx] = PTE_L2_OAB(phys) | attrs;
    }
  }

  return (uintptr_t)l0_identity_table;
}

uintptr_t setup_higher_half_tables(void) {
  memset(l0_higher_half_table, 0, sizeof(l0_higher_half_table));
  memset(l1_higher_half_table, 0, sizeof(l1_higher_half_table));
  memset(l2_higher_half_tables, 0, sizeof(l2_higher_half_tables));

  l0_higher_half_table[0] =
      PTE_NLTA((uintptr_t)l1_higher_half_table) | PTE_TABLE | PTE_VALID;

  for (size_t l1_idx = 0; l1_idx < NUM_L1_PTES; l1_idx++) {
    l1_higher_half_table[l1_idx] =
        PTE_NLTA((uintptr_t)l2_higher_half_tables[l1_idx]) | PTE_TABLE |
        PTE_VALID;

    for (size_t l2_idx = 0; l2_idx < NUM_TABLE_ENTRIES; l2_idx++) {
      uintptr_t phys = (l1_idx * L1_BLOCK_SIZE) + (l2_idx * L2_BLOCK_SIZE);

      uint64_t attrs = PTE_AF | PTE_VALID;
      if (phys_is_device(phys)) {
        attrs |= PTE_ATTRINDX(0) | PTE_UXN | PTE_PXN;
      } else {
        attrs |= PTE_ATTRINDX(1) | PTE_SH_INNER;
      }

      l2_higher_half_tables[l1_idx][l2_idx] = PTE_L2_OAB(phys) | attrs;
    }
  }

  return (uintptr_t)l0_higher_half_table;
}

uintptr_t alloc_page_table(void) {
  struct page* p = alloc_page();
  if (!p) {
    return 0;
  }

  memset(kmap(p), 0, PAGE_SIZE);

  return page_to_phys(p);
}

int map_page_l3(uint64_t* l0_table, uint64_t va, uint64_t phys,
                uint64_t attrs) {
  uint64_t* l3_table = walk_to_l3(l0_table, va, true);
  if (!l3_table) {
    return -1;
  }

  uint64_t l3_idx = L3_INDEX(va);
  l3_table[l3_idx] = PTE_L3_OAB(phys) | attrs | PTE_VALID | PTE_PAGE;

  tlb_flush_addr(va);

  return 0;
}

void unmap_page_l3(uint64_t* l0_table, uint64_t va) {
  uint64_t* l3_table = walk_to_l3(l0_table, va, false);
  if (!l3_table) {
    return;
  }

  uint64_t l3_idx = L3_INDEX(va);
  l3_table[l3_idx] &= ~PTE_VALID;

  tlb_flush_addr(va);
}

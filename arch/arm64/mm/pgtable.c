#include "mm/pgtable.h"

#include <stddef.h>
#include <stdint.h>

#include "mm/kmap.h"
#include "mm/page.h"
#include "mm/tlb.h"

#define L0_INDEX(va) (((va) >> 39) & 0x1FF)
#define L1_INDEX(va) (((va) >> 30) & 0x1FF)
#define L2_INDEX(va) (((va) >> 21) & 0x1FF)
#define L3_INDEX(va) (((va) >> 12) & 0x1FF)

#define PTE_ADDR(pte) ((pte) & 0x0000FFFFFFFFF000ULL)
#define PTE_IS_VALID(pte) ((pte) & PTE_VALID)
#define PTE_IS_TABLE(pte) (((pte) & 0x3) == 0x3)

static uint64_t alloc_page_table(void) {
  struct page* p = alloc_page();
  if (!p) {
    return 0;
  }

  // TODO: Switch to memset
  uint64_t* table = (uint64_t*)kmap(p);
  for (int i = 0; i < PTRS_PER_TABLE; i++) {
    table[i] = 0;
  }

  return page_to_phys(p);
}

// Walk page tables and ensure L3 table exists
// Returns virtual address of L3 table, or NULL on failure
static uint64_t* get_or_create_l3_table(uint64_t* l0_table, uint64_t va) {
  // L0 -> L1
  uint64_t l0_idx = L0_INDEX(va);
  uint64_t l0_entry = l0_table[l0_idx];

  uint64_t* l1_table;
  if (!PTE_IS_VALID(l0_entry)) {
    // Should only happen for user
    uint64_t l1_phys = alloc_page_table();
    if (!l1_phys) {
      return NULL;
    }

    l0_table[l0_idx] = l1_phys | PTE_VALID | PTE_TABLE;
    l1_table = (uint64_t*)(KERNEL_BASE + l1_phys);
  } else {
    uint64_t l1_phys = PTE_ADDR(l0_entry);
    l1_table = (uint64_t*)(KERNEL_BASE + l1_phys);
  }

  // L1 -> L2
  uint64_t l1_idx = L1_INDEX(va);
  uint64_t l1_entry = l1_table[l1_idx];

  uint64_t* l2_table;
  if (!PTE_IS_VALID(l1_entry)) {
    // Allocate new L2 table
    uint64_t l2_phys = alloc_page_table();
    if (!l2_phys) {
      return NULL;
    }

    l1_table[l1_idx] = l2_phys | PTE_VALID | PTE_TABLE;
    l2_table = (uint64_t*)(KERNEL_BASE + l2_phys);
  } else if (PTE_IS_TABLE(l1_entry)) {
    uint64_t l2_phys = PTE_ADDR(l1_entry);
    l2_table = (uint64_t*)(KERNEL_BASE + l2_phys);
  } else {
    // L1 is a 1 GiB block - can't create L2 here
    return NULL;
  }

  // L2 -> L3
  uint64_t l2_idx = L2_INDEX(va);
  uint64_t l2_entry = l2_table[l2_idx];

  uint64_t* l3_table;
  if (!PTE_IS_VALID(l2_entry)) {
    // Allocate new L3 table
    uint64_t l3_phys = alloc_page_table();
    if (!l3_phys) {
      return NULL;
    }

    l2_table[l2_idx] = l3_phys | PTE_VALID | PTE_TABLE;
    l3_table = (uint64_t*)(KERNEL_BASE + l3_phys);
  } else if (PTE_IS_TABLE(l2_entry)) {
    uint64_t l3_phys = PTE_ADDR(l2_entry);
    l3_table = (uint64_t*)(KERNEL_BASE + l3_phys);
  } else {
    // L2 is a 2 MiB block - can't create L3 here
    return NULL;
  }

  return l3_table;
}

int map_page_l3(uint64_t* l0_table, uint64_t va, uint64_t phys,
                uint64_t attrs) {
  uint64_t* l3_table = get_or_create_l3_table(l0_table, va);
  if (!l3_table) {
    return -1;
  }

  uint64_t l3_idx = L3_INDEX(va);
  l3_table[l3_idx] =
      (phys & 0x0000FFFFFFFFF000ULL) | attrs | PTE_VALID | PTE_PAGE;

  tlb_flush_addr(va);

  return 0;
}

void unmap_page_l3(uint64_t* l0_table, uint64_t va) {
  uint64_t* l3_table = get_or_create_l3_table(l0_table, va);
  if (!l3_table) {
    return;
  }

  uint64_t l3_idx = L3_INDEX(va);
  l3_table[l3_idx] = 0;

  tlb_flush_addr(va);
}

// Map page in user space (works with any L0 table)
int map_user_page(uint64_t* l0_table_phys, uint64_t va, uint64_t phys,
                  uint64_t attrs) {
  // For user mappings, we need to work with physical addresses
  struct page* pgd_page = phys_to_page((uint64_t)l0_table_phys);
  if (!pgd_page) {
    return -1;  // Invalid physical address
  }

  uint64_t* l0_table = (uint64_t*)kmap(pgd_page);
  return map_page_l3(l0_table, va, phys, attrs);
}

void switch_user_pgd(uint64_t* pgd_phys) {
  if (pgd_phys) {
    // Switch to user page table
    asm volatile("msr ttbr0_el1, %0" ::"r"((uint64_t)pgd_phys) : "memory");
  } else {
    // No user space, set TTBR0 to 0
    asm volatile("msr ttbr0_el1, %0" ::"r"(0ULL) : "memory");
  }
  asm volatile("tlbi vmalle1is" ::: "memory");
  asm volatile("dsb ish" ::: "memory");
  asm volatile("isb" ::: "memory");
}

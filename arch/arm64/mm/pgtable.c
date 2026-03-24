#include "mm/pgtable.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "kernel/printk.h"
#include "kernel/string.h"
#include "mm/kmap.h"
#include "mm/page.h"
#include "tlb.h"

#define L0_INDEX(va) (((va) >> 39) & 0x1FF)
#define L1_INDEX(va) (((va) >> 30) & 0x1FF)
#define L2_INDEX(va) (((va) >> 21) & 0x1FF)
#define L3_INDEX(va) (((va) >> 12) & 0x1FF)

#define PTE_TYPE_MASK 0x3
#define PTE_TYPE_TABLE 0x3
#define PTE_TYPE_BLOCK 0x1
#define PTE_ADDR_MASK 0x0000FFFFFFFFF000ULL

#define PTE_ADDR(pte) ((pte) & PTE_ADDR_MASK)
#define PTE_IS_VALID(pte) ((pte) & PTE_VALID)
#define PTE_IS_TABLE(pte) (((pte) & PTE_TYPE_MASK) == PTE_TYPE_TABLE)

static uint64_t alloc_page_table(void) {
  struct page* p = alloc_page();
  if (!p) {
    return 0;
  }

  memset(kmap(p), 0, PTRS_PER_TABLE);

  return page_to_phys(p);
}

static uint64_t* get_or_create_l3_table(uint64_t* l0_table, uint64_t va) {
  // L0 -> L1
  uint64_t l0_idx = L0_INDEX(va);
  uint64_t l0_entry = l0_table[l0_idx];

  uint64_t* l1_table;
  if (!PTE_IS_VALID(l0_entry)) {
    uint64_t l1_phys = alloc_page_table();
    if (!l1_phys) {
      return NULL;
    }

    l0_table[l0_idx] = l1_phys | PTE_VALID | PTE_TABLE;
    l1_table = (uint64_t*)(KERNEL_VIRT_BASE + l1_phys);
  } else {
    uint64_t l1_phys = PTE_ADDR(l0_entry);
    l1_table = (uint64_t*)(KERNEL_VIRT_BASE + l1_phys);
  }

  // L1 -> L2
  uint64_t l1_idx = L1_INDEX(va);
  uint64_t l1_entry = l1_table[l1_idx];

  uint64_t* l2_table;
  if (!PTE_IS_VALID(l1_entry)) {
    uint64_t l2_phys = alloc_page_table();
    if (!l2_phys) {
      return NULL;
    }

    l1_table[l1_idx] = l2_phys | PTE_VALID | PTE_TABLE;
    l2_table = (uint64_t*)(KERNEL_VIRT_BASE + l2_phys);
  } else if (PTE_IS_TABLE(l1_entry)) {
    uint64_t l2_phys = PTE_ADDR(l1_entry);
    l2_table = (uint64_t*)(KERNEL_VIRT_BASE + l2_phys);
  } else {
    // L1 is a 1 GiB block
    return NULL;
  }

  // L2 -> L3
  uint64_t l2_idx = L2_INDEX(va);
  uint64_t l2_entry = l2_table[l2_idx];

  uint64_t* l3_table;
  if (!PTE_IS_VALID(l2_entry)) {
    uint64_t l3_phys = alloc_page_table();
    if (!l3_phys) {
      return NULL;
    }

    l2_table[l2_idx] = l3_phys | PTE_VALID | PTE_TABLE;
    l3_table = (uint64_t*)(KERNEL_VIRT_BASE + l3_phys);
  } else if (PTE_IS_TABLE(l2_entry)) {
    uint64_t l2_phys = PTE_ADDR(l2_entry);
    l3_table = (uint64_t*)(KERNEL_VIRT_BASE + l2_phys);
  } else {
    // L2 is a 2 MiB block
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
  l3_table[l3_idx] = (phys & PTE_ADDR_MASK) | attrs | PTE_VALID | PTE_PAGE;

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

int map_user_page(uint64_t* l0_table_phys, uint64_t va, uint64_t phys,
                  uint64_t attrs) {
  struct page* pgd_page = phys_to_page((uint64_t)l0_table_phys);
  if (!pgd_page) {
    return -1;
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
  tlb_flush_all();
}

static int copy_page_contents(uint64_t dest_phys, uint64_t src_phys) {
  void* dest = kmap(phys_to_page(dest_phys));
  void* src = kmap(phys_to_page(src_phys));

  if (!dest || !src) {
    return -1;
  }

  memcpy(dest, src, PAGE_SIZE);
  return 0;
}

// TODO: Make iterative
static uint64_t* copy_page_table_level(uint64_t* src_table, int level);

static int copy_pte(uint64_t* dest_table, uint64_t* src_table, int index,
                    int level) {
  uint64_t src_pte = src_table[index];

  if (!PTE_IS_VALID(src_pte)) {
    dest_table[index] = 0;
    return 0;
  }

  if (level == 3 || !PTE_IS_TABLE(src_pte)) {
    // This is a page mapping, need to allocate new page and copy contents
    struct page* new_page = alloc_page();
    if (!new_page) {
      printk("copy_pte: Failed to allocate page\n");
      return -1;
    }

    uint64_t new_phys = page_to_phys(new_page);
    uint64_t src_phys = PTE_ADDR(src_pte);

    if (copy_page_contents(new_phys, src_phys) != 0) {
      free_page(new_page);
      return -1;
    }

    uint64_t new_pte = (src_pte & ~PTE_ADDR_MASK) | (new_phys & PTE_ADDR_MASK);
    dest_table[index] = new_pte;

    return 0;
  }

  uint64_t src_table_phys = PTE_ADDR(src_pte);
  uint64_t* src_next_table = (uint64_t*)kmap(phys_to_page(src_table_phys));

  uint64_t* new_next_table = copy_page_table_level(src_next_table, level + 1);
  if (!new_next_table) {
    return -1;
  }

  uint64_t new_table_phys = ((uint64_t)new_next_table) - KERNEL_VIRT_BASE;

  uint64_t new_pte =
      (new_table_phys & PTE_ADDR_MASK) | PTE_TYPE_TABLE | PTE_VALID;
  dest_table[index] = new_pte;

  return 0;
}

static uint64_t* copy_page_table_level(uint64_t* src_table, int level) {
  if (!src_table) {
    return NULL;
  }

  struct page* table_page = alloc_page();
  if (!table_page) {
    printk("copy_page_table_level: Failed to allocate table page\n");
    return NULL;
  }

  uint64_t* dest_table = (uint64_t*)kmap(table_page);
  memset(dest_table, 0, PAGE_SIZE);

  int num_entries = 512;
  for (int i = 0; i < num_entries; i++) {
    if (PTE_IS_VALID(src_table[i])) {
      if (copy_pte(dest_table, src_table, i, level) != 0) {
        // TODO: Cleanup already copied entries
        free_page(table_page);
        return NULL;
      }
    }
  }

  return dest_table;
}

uint64_t* copy_user_pgd(uint64_t* src_pgd) {
  if (!src_pgd) {
    return NULL;
  }

  printk("Copying user page tables from %lx\n", (uint64_t)src_pgd);
  uint64_t* new_pgd = copy_page_table_level(src_pgd, 0);

  if (new_pgd) {
    printk("Successfully copied user page tables to %lx\n", (uint64_t)new_pgd);
  } else {
    printk("Failed to copy user page tables\n");
  }

  return new_pgd;
}

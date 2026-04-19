#include "mm/user_pgtable.h"

#include <stddef.h>
#include <stdint.h>

#include "kernel/string.h"
#include "mm/kmap.h"
#include "mm/page.h"
#include "mm/pgtable.h"

static int copy_page_contents(uintptr_t dest_phys, uintptr_t src_phys) {
  void* dest = kmap(phys_to_page(dest_phys));
  void* src = kmap(phys_to_page(src_phys));

  if (!dest || !src) {
    return -1;
  }

  if (dest == src) {
    return 0;
  }

  memcpy(dest, src, PAGE_SIZE);

  return 0;
}

static uint64_t* copy_page_table_level(uint64_t* src_table, int level);

static int copy_pte(uint64_t* dest_table, uint64_t* src_table, int index,
                    int level) {
  uint64_t src_pte = src_table[index];

  if (level == 3 || !PTE_IS_TABLE(src_pte)) {
    struct page* new_page = alloc_page();
    if (!new_page) {
      return -1;
    }

    uintptr_t new_phys = page_to_phys(new_page);
    uintptr_t src_phys = PTE_L3_OAB(src_pte);

    if (copy_page_contents(new_phys, src_phys) != 0) {
      free_page(new_page);
      return -1;
    }

    uint64_t new_pte =
        (src_pte & ~PTE_L3_OAB_MASK) | (new_phys & PTE_L3_OAB_MASK);
    dest_table[index] = new_pte;

    return 0;
  }

  uint64_t src_table_phys = PTE_L3_OAB(src_pte);
  uint64_t* src_next_table = (uint64_t*)kmap(phys_to_page(src_table_phys));

  uint64_t* new_next_table = copy_page_table_level(src_next_table, level + 1);
  if (!new_next_table) {
    return -1;
  }

  uint64_t new_table_phys = kernel_to_phys((uintptr_t)new_next_table);

  uint64_t new_pte =
      (new_table_phys & PTE_NLTA_MASK) | PTE_TYPE_TABLE | PTE_VALID;
  dest_table[index] = new_pte;

  return 0;
}

static uint64_t* copy_page_table_level(uint64_t* src_table, int level) {
  if (!src_table) {
    return NULL;
  }

  struct page* table_page = alloc_page();
  if (!table_page) {
    return NULL;
  }

  uint64_t* dest_table = (uint64_t*)kmap(table_page);
  memset(dest_table, 0, PAGE_SIZE);

  for (int i = 0; i < NUM_TABLE_ENTRIES; i++) {
    if (PTE_IS_VALID(src_table[i])) {
      if (copy_pte(dest_table, src_table, i, level) != 0) {
        free_page_table(page_to_phys(table_page), level);
        return NULL;
      }
    }
  }

  return dest_table;
}

uintptr_t copy_user_pgd(uint64_t* src_pgd) {
  if (!src_pgd) {
    return 0;
  }

  return (uintptr_t)copy_page_table_level(src_pgd, 0);
}

void free_page_table(uintptr_t table_phys, uint8_t level) {
  if (!table_phys || level > 3) {
    return;
  }

  uint64_t* table = (uint64_t*)kmap(phys_to_page(table_phys));

  if (level < 3) {
    for (int i = 0; i < NUM_TABLE_ENTRIES; i++) {
      uint64_t entry = table[i];
      if ((entry & PTE_VALID) && PTE_IS_TABLE(entry)) {
        uint64_t child_phys = PTE_NLTA(entry);
        free_page_table(child_phys, level + 1);
      }
    }
  }

  free_page(phys_to_page(table_phys));
}

void free_user_pgd(uintptr_t pgd_phys) {
  if (!pgd_phys) {
    return;
  }

  free_page_table(pgd_phys, 0);
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

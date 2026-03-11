#include "mm/user_pgtable.h"

#include <stddef.h>
#include <stdint.h>

#include "kernel/string.h"
#include "mm/kmap.h"
#include "mm/mmu.h"
#include "mm/page.h"
#include "mm/pgtable.h"

uint64_t* alloc_user_pgd(void) {
  struct page* p = alloc_page();
  if (!p) {
    return NULL;
  }

  uint64_t* table = (uint64_t*)kmap(p);
  memset(table, 0, PAGE_SIZE);

  uint64_t* kernel_l0 = (uint64_t*)get_kernel_l0_table();

  return (uint64_t*)page_to_phys(p);
}

static void free_page_table_recursive(uint64_t* table_phys, int level) {
  if (!table_phys || level > 3) {
    return;
  }

  uint64_t* table = (uint64_t*)kmap(phys_to_page((uint64_t)table_phys));

  if (level < 3) {
    for (int i = 0; i < PTRS_PER_TABLE; i++) {
      uint64_t entry = table[i];
      if ((entry & PTE_VALID) && ((entry & 0x3) == 0x3)) {
        uint64_t child_phys = entry & 0x0000FFFFFFFFF000ULL;
        free_page_table_recursive((uint64_t*)child_phys, level + 1);
      }
    }
  }

  free_page(phys_to_page((uint64_t)table_phys));
}

void free_user_pgd(uint64_t* pgd_phys) {
  if (!pgd_phys) {
    return;
  }

  free_page_table_recursive(pgd_phys, 0);
}

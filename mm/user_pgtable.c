#include "mm/user_pgtable.h"

#include <stddef.h>
#include <stdint.h>

#include "mm/kmap.h"
#include "mm/page.h"
#include "mm/pgtable.h"

static void free_page_table(uintptr_t table_phys, uint8_t level) {
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

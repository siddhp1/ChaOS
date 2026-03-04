#include "kernel/vm.h"

#include <stddef.h>
#include <stdint.h>

#include "kernel/printk.h"
#include "kernel/string.h"
#include "kernel/task.h"
#include "mm/kmap.h"
#include "mm/mmu.h"
#include "mm/page.h"
#include "mm/pgtable.h"

uint64_t vm_create_user_ttbr0(void) { return mmu_create_user_ttbr0(); }

void vm_map_user_page(uint64_t ttbr0, uint64_t va, struct page *page,
                      uint64_t flags) {
  struct page *l0_page = phys_to_page(ttbr0);
  uint64_t *l0 = (uint64_t *)kmap(l0_page);

  struct page *l1_page = phys_to_page(l0[0] & 0x0000FFFFFFFFF000ULL);
  uint64_t *l1 = (uint64_t *)kmap(l1_page);

  int idx = (va >> 21) & 0x1FF;

  l1[idx] =
      (page_to_phys(page) & 0x0000FFFFFFFFF000ULL) | flags | PTE_VALID | PTE_AF;
}

void vm_destroy_user(struct task *t) {
  if (!t->ttbr0) return;

  for (uint64_t i = 0; i < t->user_page_count; i++) {
    if (t->user_pages[i]) {
      free_page(t->user_pages[i]);
      t->user_pages[i] = NULL;
    }
  }
  t->user_page_count = 0;

  struct page *l0_page = phys_to_page(t->ttbr0);
  if (l0_page) {
    uint64_t *l0 = (uint64_t *)kmap(l0_page);

    if (l0[0] & PTE_VALID) {
      struct page *l1_page = phys_to_page(l0[0] & 0x0000FFFFFFFFF000ULL);
      if (l1_page) {
        free_page(l1_page);
      }
    }

    free_page(l0_page);
  }

  t->ttbr0 = 0;
}

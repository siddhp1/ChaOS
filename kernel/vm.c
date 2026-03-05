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

#define PTE_ADDR_MASK 0x0000FFFFFFFFF000ULL

static uint64_t *get_table(uint64_t entry) {
  struct page *p = phys_to_page(entry & PTE_ADDR_MASK);
  return p ? (uint64_t *)kmap(p) : NULL;
}

static uint64_t *ensure_table(uint64_t *parent, int idx) {
  if (parent[idx] & PTE_VALID) {
    return get_table(parent[idx]);
  }

  struct page *p = alloc_page();
  if (!p) return NULL;

  uint64_t *tbl = (uint64_t *)kmap(p);
  memset(tbl, 0, PAGE_SIZE);
  parent[idx] = (page_to_phys(p) & PTE_ADDR_MASK) | PTE_VALID | PTE_TABLE;
  return tbl;
}

void vm_map_user_page(uint64_t ttbr0, uint64_t va, struct page *page,
                      uint64_t flags) {
  struct page *l0_page = phys_to_page(ttbr0);
  uint64_t *l0 = (uint64_t *)kmap(l0_page);

  int l0_idx = (va >> 39) & 0x1FF;
  int l1_idx = (va >> 30) & 0x1FF;
  int l2_idx = (va >> 21) & 0x1FF;
  int l3_idx = (va >> 12) & 0x1FF;

  uint64_t *l1 = ensure_table(l0, l0_idx);
  if (!l1) return;

  uint64_t *l2 = ensure_table(l1, l1_idx);
  if (!l2) return;

  uint64_t *l3 = ensure_table(l2, l2_idx);
  if (!l3) return;

  /* L3 page descriptor: bits[1:0] = 0b11 (PTE_VALID | PTE_TABLE) */
  l3[l3_idx] = (page_to_phys(page) & PTE_ADDR_MASK) | flags | PTE_VALID |
               PTE_TABLE | PTE_AF | PTE_SH_INNER | PTE_ATTRINDX(1);
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
  if (!l0_page) {
    t->ttbr0 = 0;
    return;
  }
  uint64_t *l0 = (uint64_t *)kmap(l0_page);

  for (int i0 = 0; i0 < PTRS_PER_TABLE; i0++) {
    if ((l0[i0] & (PTE_VALID | PTE_TABLE)) != (PTE_VALID | PTE_TABLE)) continue;
    struct page *l1_page = phys_to_page(l0[i0] & PTE_ADDR_MASK);
    if (!l1_page) continue;
    uint64_t *l1 = (uint64_t *)kmap(l1_page);

    for (int i1 = 0; i1 < PTRS_PER_TABLE; i1++) {
      if ((l1[i1] & (PTE_VALID | PTE_TABLE)) != (PTE_VALID | PTE_TABLE))
        continue;
      struct page *l2_page = phys_to_page(l1[i1] & PTE_ADDR_MASK);
      if (!l2_page) continue;
      uint64_t *l2 = (uint64_t *)kmap(l2_page);

      for (int i2 = 0; i2 < PTRS_PER_TABLE; i2++) {
        if ((l2[i2] & (PTE_VALID | PTE_TABLE)) != (PTE_VALID | PTE_TABLE))
          continue;
        struct page *l3_page = phys_to_page(l2[i2] & PTE_ADDR_MASK);
        if (l3_page) free_page(l3_page);
      }

      free_page(l2_page);
    }

    free_page(l1_page);
  }

  free_page(l0_page);
  t->ttbr0 = 0;
}

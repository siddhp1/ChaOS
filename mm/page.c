#include "mm/page.h"

#include <stddef.h>

#include "kernel/printk.h"
#include "mm/kmap.h"
#include "mm/page.h"

#define PHYS_START 0x40000000UL
#define PHYS_END 0x60000000UL
#define TOTAL_PAGES (PHYS_END - PHYS_START) >> PAGE_SHIFT

extern char page_start[];

static struct page_internal pages[TOTAL_PAGES];

struct page_internal* free_list;

void page_init(void) {
  uintptr_t page_start_phys = kernel_to_phys((uintptr_t)page_start);

  size_t first_free_idx = 0;
  first_free_idx = (page_start_phys - PHYS_START) >> PAGE_SHIFT;
  if (first_free_idx > TOTAL_PAGES) {
    first_free_idx = TOTAL_PAGES;
  }

  for (size_t i = 0; i < first_free_idx; i++) {
    pages[i].pub.refcount = 1;
    pages[i].pub.flags = 0;
    pages[i].next = NULL;
  }

  free_list = NULL;
  for (size_t i = first_free_idx; i < TOTAL_PAGES; i++) {
    pages[i].pub.flags = 0;
    pages[i].pub.refcount = 0;
    pages[i].next = free_list;
    free_list = &pages[i];
  }
}

struct page* alloc_page(void) {
  if (!free_list) return NULL;

  struct page_internal* ptr = free_list;
  free_list = ptr->next;
  ptr->next = NULL;
  ptr->pub.refcount = 1;
  return &ptr->pub;
}

void free_page(struct page* page) {
  if (!page) return;

  struct page_internal* ptr = (struct page_internal*)page;
  ptr->pub.refcount = 0;
  ptr->next = free_list;
  free_list = ptr;
}

uintptr_t page_to_phys(struct page* page) {
  struct page_internal* p = (struct page_internal*)page;
  size_t idx = (size_t)(p - pages);
  return PHYS_START + (idx << PAGE_SHIFT);
}

struct page* phys_to_page(uintptr_t phys) {
  if (phys < PHYS_START || phys >= PHYS_END) {
    return NULL;
  }

  size_t idx = (phys - PHYS_START) >> PAGE_SHIFT;
  return &pages[idx].pub;
}

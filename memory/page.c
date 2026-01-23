#include "memory/page.h"

#include <stddef.h>

#include "memory/physical.h"

static struct page_internal
    pages[(PHYSICAL_END - PHYSICAL_START) >> PAGE_SHIFT];
static size_t total_pages;

struct page_internal* free_list;

void page_init(void) {
  total_pages = (physical_end - physical_start) >> PAGE_SHIFT;

  free_list = NULL;
  for (size_t i = 0; i < total_pages; i++) {
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

uintptr_t page_to_physical(struct page* page) {
  struct page_internal* p = (struct page_internal*)page;
  size_t idx = (size_t)(p - pages);
  return physical_start + (idx << PAGE_SHIFT);
}

struct page* physical_to_page(uintptr_t phys) {
  if (phys < physical_start || phys >= physical_end) {
    return NULL;
  }

  size_t idx = (phys - physical_start) >> PAGE_SHIFT;
  return &pages[idx].pub;
}

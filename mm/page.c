#include "mm/page.h"

#include <stddef.h>

#include "kernel/printk.h"
#include "page_internal.h"
#include "phys.h"

static struct page_internal pages[(PHYS_END - PHYS_START) >> PAGE_SHIFT];
static size_t total_pages;

struct page_internal* free_list;

void page_init(void) {
  total_pages = (phys_end - phys_start) >> PAGE_SHIFT;

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

void page_get(struct page* page) {
  if (page && page->refcount > 0) {
    page->refcount++;
  }
}

void page_put(struct page* page) {
  if (!page || page->refcount == 0) return;

  page->refcount--;
  if (page->refcount == 0) {
    free_page(page);
  }
}

uintptr_t page_to_phys(struct page* page) {
  struct page_internal* p = (struct page_internal*)page;
  size_t idx = (size_t)(p - pages);
  return phys_start + (idx << PAGE_SHIFT);
}

struct page* phys_to_page(uintptr_t phys) {
  if (phys < phys_start || phys >= phys_end) {
    return NULL;
  }

  size_t idx = (phys - phys_start) >> PAGE_SHIFT;
  return &pages[idx].pub;
}

void dump_free_pages(void) {
  size_t count = 0;
  for (struct page_internal* p = free_list; p; p = p->next) {
    count++;
  }

  printk("Free pages: %lu / %lu\n", (uint64_t)count, (uint64_t)total_pages);
}

void dump_page(struct page* page) {
  if (!page) {
    printk("Page: (null)\n");
    return;
  }

  uintptr_t phys = page_to_phys(page);
  printk("Page: phys=%lu flags=%u refcount=%u\n", (uint64_t)phys, page->flags,
         page->refcount);
}

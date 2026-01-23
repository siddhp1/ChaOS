#ifndef PAGE_H
#define PAGE_H

#include <stdint.h>

// 4 KiB page size
#define PAGE_SHIFT 12
#define PAGE_SIZE (1UL << PAGE_SHIFT)

struct page {
  uint32_t flags;
  uint32_t refcount;
};

struct page* alloc_page(void);
void free_page(struct page*);

struct page_internal {
  struct page pub;
  struct page_internal* next;
};

extern struct page_internal* free_list;

uintptr_t page_to_physical(struct page* page);
struct page* physical_to_page(uintptr_t phys);

void page_init(void);

void dump_free_pages(void);
void dump_page(struct page*);

#endif

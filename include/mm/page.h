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

uintptr_t page_to_phys(struct page* page);
struct page* phys_to_page(uintptr_t phys);

void dump_free_pages(void);
void dump_page(struct page* page);

#endif

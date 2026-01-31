#ifndef PAGE_INTERNAL_H
#define PAGE_INTERNAL_H

#include "mm/page.h"

struct page_internal {
  struct page pub;
  struct page_internal* next;
};

extern struct page_internal* free_list;

void page_init(void);

#endif

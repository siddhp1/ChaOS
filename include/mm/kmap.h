#ifndef KMAP_H
#define KMAP_H

#include "mm/page.h"

#define KERNEL_BASE 0xFFFF000000000000UL

void* kmap(struct page* page);

#endif

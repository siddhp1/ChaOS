#ifndef KMAP_H
#define KMAP_H

#include "mm/page.h"

#define KERNEL_VIRT_BASE 0xFFFF000000000000UL

void* kmap(struct page* page);
uintptr_t kernel_to_phys(uintptr_t va);

#endif

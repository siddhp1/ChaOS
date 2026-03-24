#include "mm/kmap.h"

#include <stdint.h>

#include "mm/page.h"

void* kmap(struct page* page) {
  uintptr_t phys = page_to_phys(page);
  return (void*)(KERNEL_VIRT_BASE + phys);
}

uintptr_t kernel_to_phys(uintptr_t va) { return va - KERNEL_VIRT_BASE; }

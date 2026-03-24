#include "mm/kmap.h"

#include <stdint.h>

#include "mm/page.h"

void* kmap(struct page* page) {
  uintptr_t phys = page_to_phys(page);
  return (void*)(KERNEL_VIRT_BASE + phys);
}

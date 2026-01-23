#include "memory/kmap.h"

#include <stdint.h>

#include "memory/page.h"

void* kmap(struct page* page) {
  uintptr_t physical = page_to_physical(page);
  return (void*)(KERNEL_BASE + physical);
}

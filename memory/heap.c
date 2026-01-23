#include "memory/heap.h"

#include "memory/kmap.h"
#include "memory/page.h"

void* kmalloc(size_t size) {
  struct page* page = alloc_page();
  return kmap(page);
}

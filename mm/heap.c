#include "mm/heap.h"

#include "mm/kmap.h"
#include "mm/page.h"

void* kmalloc(size_t size) {
  struct page* page = alloc_page();
  return kmap(page);
}

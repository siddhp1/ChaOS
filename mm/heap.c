#include "mm/heap.h"

#include "kernel/string.h"
#include "mm/slab.h"

void* kmalloc(size_t size) { return slab_alloc(size); }
void* kzalloc(size_t size) {
  void* mem = slab_alloc(size);
  if (mem == NULL) return NULL;
  return memset(mem, 0, size);
}

void kfree(void* ptr) { slab_free(ptr); }

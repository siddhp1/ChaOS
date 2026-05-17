#include "mm/heap.h"

#include "mm/slab.h"

void* kmalloc(size_t size) { return slab_alloc(size); }
void kfree(void* ptr) { slab_free(ptr); }

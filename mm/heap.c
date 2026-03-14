#include "mm/heap.h"

#include "mm/kvmalloc.h"

void* kmalloc(size_t size) { return kvmalloc(size); }

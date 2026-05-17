#ifndef HEAP_H
#define HEAP_H

#include <stddef.h>

void* kmalloc(size_t size);
void* kzalloc(size_t size);

void kfree(void* ptr);

#endif

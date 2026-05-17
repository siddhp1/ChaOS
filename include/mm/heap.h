#ifndef HEAP_H
#define HEAP_H

#include <stddef.h>

void* kmalloc(size_t size);
void kfree(void* ptr);

// TODO: Add kzalloc (zero-initializes allocated memory)
// https://www.kernel.org/doc/html/v5.8/core-api/mm-api.html#c.kzalloc

#endif

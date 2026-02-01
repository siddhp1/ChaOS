#ifndef PHYS_H
#define PHYS_H

#include <stdint.h>

#define PHYS_START 0x40000000UL
#define PHYS_END 0x60000000UL

extern uintptr_t phys_start;
extern uintptr_t phys_end;

void phys_init(void);

#endif

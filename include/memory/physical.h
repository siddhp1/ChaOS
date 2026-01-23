#ifndef PHYSICAL_H
#define PHYSICAL_H

#include <stdint.h>

#define PHYSICAL_START 0x40000000UL
#define PHYSICAL_END 0x60000000UL

extern uintptr_t physical_start;
extern uintptr_t physical_end;

void phys_init(void);

#endif

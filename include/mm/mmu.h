#ifndef MMU_H
#define MMU_H

#include <stdint.h>

void mmu_init(void);

void set_ttbr0(uintptr_t phys);

#endif

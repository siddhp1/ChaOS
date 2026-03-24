#ifndef MMU_H
#define MMU_H

#include <stdint.h>

uint64_t* get_kernel_l0_table(void);
void mmu_init(void);

#endif

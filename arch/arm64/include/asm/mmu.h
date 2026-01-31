#ifndef MMU_H
#define MMU_H

#include <stdint.h>

#define KERNEL_BASE 0xFFFF000000000000UL
#define PAGE_SIZE_2MB (2UL << 20)
#define NUM_KERNEL_BLOCKS 1024

#define MAIR_ATTR_DEVICE 0x00
#define MAIR_ATTR_NORMAL 0xff
#define MAIR_VALUE ((MAIR_ATTR_DEVICE << 0) | (MAIR_ATTR_NORMAL << 8))
#define TCR_T0SZ (64 - 48)
#define TCR_T1SZ (64 - 48)
#define TCR_VALUE \
  ((TCR_T0SZ << 0) | (TCR_T1SZ << 16) | (0b00 << 14) | (0b10 << 30))

void setup_page_tables(void);
void enable_mmu(uintptr_t ttbr0, uintptr_t ttbr1);
void mmu_init(void);

#endif

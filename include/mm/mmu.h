#ifndef MMU_H
#define MMU_H

#include <stdint.h>

#define KERNEL_BASE 0xFFFF000000000000UL

#define L1_BLOCK_SIZE (1UL << 30)
#define L2_BLOCK_SIZE (1UL << 21)

#define PHYS_DEVICE_BASE 0x00000000UL
#define PHYS_KERNEL_BASE 0x40000000UL
#define UART_PHYS_BASE 0x09000000UL  // PL011 UART

#define VIRT_DEVICE_BASE (KERNEL_BASE + PHYS_DEVICE_BASE)
#define VIRT_KERNEL_BASE (KERNEL_BASE + PHYS_KERNEL_BASE)
#define UART_VIRT_BASE (KERNEL_BASE + UART_PHYS_BASE)

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

uintptr_t mmu_kernel_ttbr0(void);
uintptr_t mmu_create_user_ttbr0(void);
void mmu_switch_ttbr0(uintptr_t ttbr0);
uintptr_t mmu_current_user_ttbr0(void);

#endif

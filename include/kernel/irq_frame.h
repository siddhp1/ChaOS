#ifndef IRQ_FRAME_H
#define IRQ_FRAME_H

// IRQ stack frame layout (18 x 16-byte slots)
#define IRQ_FRAME_SIZE (16 * 18)
#define IRQ_OFF_X0_X1 (16 * 0)
#define IRQ_OFF_X2_X3 (16 * 1)
#define IRQ_OFF_X4_X5 (16 * 2)
#define IRQ_OFF_X6_X7 (16 * 3)
#define IRQ_OFF_X8_X9 (16 * 4)
#define IRQ_OFF_X10_X11 (16 * 5)
#define IRQ_OFF_X12_X13 (16 * 6)
#define IRQ_OFF_X14_X15 (16 * 7)
#define IRQ_OFF_X16_X17 (16 * 8)
#define IRQ_OFF_X18_X19 (16 * 9)
#define IRQ_OFF_X20_X21 (16 * 10)
#define IRQ_OFF_X22_X23 (16 * 11)
#define IRQ_OFF_X24_X25 (16 * 12)
#define IRQ_OFF_X26_X27 (16 * 13)
#define IRQ_OFF_X28_X29 (16 * 14)
#define IRQ_OFF_X30 (16 * 15)
#define IRQ_OFF_ELR_SPSR (16 * 16)
#define IRQ_OFF_USER_SP (16 * 17)

#define SPSR_EL0 0x0
#define SPSR_EL1H 0x5

#endif

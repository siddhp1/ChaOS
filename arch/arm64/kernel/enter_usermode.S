.section .text
.global enter_usermode

// void enter_usermode(uint64_t pc, uint64_t sp)
// x0 = user PC
// x1 = user SP
enter_usermode:
    // Set up exception return frame on kernel stack

    // SPSR_EL1: User mode (EL0t), interrupts enabled
    // D=0, A=0, I=0, F=0 (debug, SError, IRQ, FIQ all unmasked)
    // M[4:0] = 0b00000 (EL0t)
    mov x2, #0x0
    msr SPSR_EL1, x2

    // Set user PC
    msr ELR_EL1, x0

    // Set user SP
    msr SP_EL0, x1

    // Clear all general purpose registers for security
    mov x0, #0
    mov x1, #0
    mov x2, #0
    mov x3, #0
    mov x4, #0
    mov x5, #0
    mov x6, #0
    mov x7, #0
    mov x8, #0
    mov x9, #0
    mov x10, #0
    mov x11, #0
    mov x12, #0
    mov x13, #0
    mov x14, #0
    mov x15, #0
    mov x16, #0
    mov x17, #0
    mov x18, #0
    mov x19, #0
    mov x20, #0
    mov x21, #0
    mov x22, #0
    mov x23, #0
    mov x24, #0
    mov x25, #0
    mov x26, #0
    mov x27, #0
    mov x28, #0
    mov x29, #0
    mov x30, #0

    eret

.section .text
.global vectors
.align 11

// IRQ stack frame layout (17 x 16-byte slots)
.equ IRQ_FRAME_SIZE,   (16 * 17)
.equ IRQ_OFF_X0_X1,    (16 * 0)
.equ IRQ_OFF_X2_X3,    (16 * 1)
.equ IRQ_OFF_X4_X5,    (16 * 2)
.equ IRQ_OFF_X6_X7,    (16 * 3)
.equ IRQ_OFF_X8_X9,    (16 * 4)
.equ IRQ_OFF_X10_X11,  (16 * 5)
.equ IRQ_OFF_X12_X13,  (16 * 6)
.equ IRQ_OFF_X14_X15,  (16 * 7)
.equ IRQ_OFF_X16_X17,  (16 * 8)
.equ IRQ_OFF_X18_X19,  (16 * 9)
.equ IRQ_OFF_X20_X21,  (16 * 10)
.equ IRQ_OFF_X22_X23,  (16 * 11)
.equ IRQ_OFF_X24_X25,  (16 * 12)
.equ IRQ_OFF_X26_X27,  (16 * 13)
.equ IRQ_OFF_X28_X29,  (16 * 14)
.equ IRQ_OFF_X30,      (16 * 15)
.equ IRQ_OFF_ELR_SPSR, (16 * 16)

vectors:
    // EL1t
    b sync_el1t
    .space 0x80-4
    b irq_el1t
    .space 0x80-4
    b fiq_el1t
    .space 0x80-4
    b serr_el1t
    .space 0x80-4

    // EL1h
    b sync_el1h
    .space 0x80-4
    b irq_el1h
    .space 0x80-4
    b fiq_el1h
    .space 0x80-4
    b serr_el1h
    .space 0x80-4

sync_el1t:
    bl panic_sync
    b .

irq_el1t:
    bl panic_irq
    b .

fiq_el1t:
    bl panic_fiq
    b .

serr_el1t:
    bl panic_serr
    b .

sync_el1h:
    mrs x0, ESR_EL1
    lsr x0, x0, #26

    cmp x0, #0x25
    b.eq data_abort

    cmp x0, #0x21
    b.eq inst_abort

    bl panic_sync
    b .

data_abort:
    bl data_abort_handler
    b .

inst_abort:
    bl prefetch_abort_handler
    b .

irq_el1h:
    sub sp, sp, #IRQ_FRAME_SIZE

    stp x0,  x1,  [sp, #IRQ_OFF_X0_X1]
    stp x2,  x3,  [sp, #IRQ_OFF_X2_X3]
    stp x4,  x5,  [sp, #IRQ_OFF_X4_X5]
    stp x6,  x7,  [sp, #IRQ_OFF_X6_X7]
    stp x8,  x9,  [sp, #IRQ_OFF_X8_X9]
    stp x10, x11, [sp, #IRQ_OFF_X10_X11]
    stp x12, x13, [sp, #IRQ_OFF_X12_X13]
    stp x14, x15, [sp, #IRQ_OFF_X14_X15]
    stp x16, x17, [sp, #IRQ_OFF_X16_X17]
    stp x18, x19, [sp, #IRQ_OFF_X18_X19]
    stp x20, x21, [sp, #IRQ_OFF_X20_X21]
    stp x22, x23, [sp, #IRQ_OFF_X22_X23]
    stp x24, x25, [sp, #IRQ_OFF_X24_X25]
    stp x26, x27, [sp, #IRQ_OFF_X26_X27]
    stp x28, x29, [sp, #IRQ_OFF_X28_X29]
    str x30,      [sp, #IRQ_OFF_X30]

    mrs x0, elr_el1
    mrs x1, spsr_el1
    stp x0, x1, [sp, #IRQ_OFF_ELR_SPSR]

    bl irq_handler

    ldp x0, x1, [sp, #IRQ_OFF_ELR_SPSR]
    msr elr_el1, x0
    msr spsr_el1, x1

    ldp x0,  x1,  [sp, #IRQ_OFF_X0_X1]
    ldp x2,  x3,  [sp, #IRQ_OFF_X2_X3]
    ldp x4,  x5,  [sp, #IRQ_OFF_X4_X5]
    ldp x6,  x7,  [sp, #IRQ_OFF_X6_X7]
    ldp x8,  x9,  [sp, #IRQ_OFF_X8_X9]
    ldp x10, x11, [sp, #IRQ_OFF_X10_X11]
    ldp x12, x13, [sp, #IRQ_OFF_X12_X13]
    ldp x14, x15, [sp, #IRQ_OFF_X14_X15]
    ldp x16, x17, [sp, #IRQ_OFF_X16_X17]
    ldp x18, x19, [sp, #IRQ_OFF_X18_X19]
    ldp x20, x21, [sp, #IRQ_OFF_X20_X21]
    ldp x22, x23, [sp, #IRQ_OFF_X22_X23]
    ldp x24, x25, [sp, #IRQ_OFF_X24_X25]
    ldp x26, x27, [sp, #IRQ_OFF_X26_X27]
    ldp x28, x29, [sp, #IRQ_OFF_X28_X29]
    ldr x30,      [sp, #IRQ_OFF_X30]

    add sp, sp, #IRQ_FRAME_SIZE
    eret

fiq_el1h:
    bl panic_fiq
    b .

serr_el1h:
    bl panic_serr
    b .

#include "asm/context_offsets.h"

.section .text
.global vectors
.align 11

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
    // Save all GPRs x0-x30
    // Round to 256 for alignment
    sub sp, sp, #256

    stp x0,  x1,  [sp, #0]
    stp x2,  x3,  [sp, #16]
    stp x4,  x5,  [sp, #32]
    stp x6,  x7,  [sp, #48]
    stp x8,  x9,  [sp, #64]
    stp x10, x11, [sp, #80]
    stp x12, x13, [sp, #96]
    stp x14, x15, [sp, #112]
    stp x16, x17, [sp, #128]
    stp x18, x19, [sp, #144]
    stp x20, x21, [sp, #160]
    stp x22, x23, [sp, #176]
    stp x24, x25, [sp, #192]
    stp x26, x27, [sp, #208]
    stp x28, x29, [sp, #224]
    str x30,      [sp, #240]

    bl irq_handler

    ldr x30,      [sp, #240]
    ldp x28, x29, [sp, #224]
    ldp x26, x27, [sp, #208]
    ldp x24, x25, [sp, #192]
    ldp x22, x23, [sp, #176]
    ldp x20, x21, [sp, #160]
    ldp x18, x19, [sp, #144]
    ldp x16, x17, [sp, #128]
    ldp x14, x15, [sp, #112]
    ldp x12, x13, [sp, #96]
    ldp x10, x11, [sp, #80]
    ldp x8,  x9,  [sp, #64]
    ldp x6,  x7,  [sp, #48]
    ldp x4,  x5,  [sp, #32]
    ldp x2,  x3,  [sp, #16]
    ldp x0,  x1,  [sp, #0]

    add sp, sp, #256
    eret

fiq_el1h:
    bl panic_fiq
    b .

serr_el1h:
    bl panic_serr
    b .

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
    bl panic_sync
    b .

irq_el1h:
    bl panic_irq
    b .

fiq_el1h:
    bl panic_fiq
    b .

serr_el1h:
    bl panic_serr
    b .


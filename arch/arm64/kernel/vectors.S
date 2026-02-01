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
    bl panic_irq
    b .

fiq_el1h:
    bl panic_fiq
    b .

serr_el1h:
    bl panic_serr
    b .


.section .text.boot
.global _start
.extern stack_top
.extern kernel_entry

_start:
    ldr x0, =stack_top
    mov sp, x0

    // Disable FP/NEON at EL1
    mrs x0, CPACR_EL1
    bic x0, x0, #(3 << 20)
    msr CPACR_EL1, x0
    isb

    bl kernel_entry

1:  wfi
    b 1b

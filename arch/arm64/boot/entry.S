.section .text.boot
.global _start
.extern stack_top
.extern kernel_entry

_start:
    ldr x0, =stack_top
    mov sp, x0

    bl kernel_entry

1:  wfi
    b 1b

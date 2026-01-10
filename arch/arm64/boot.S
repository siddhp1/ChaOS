.section .text.boot
.global _start

_start:
    // Set up a temporary stack
    LDR x0, =_stack_top
    MOV sp, x0

    // Jump to C code
    bl kernel_main

// Infinite loop (wait for event, branches back to label 1 backwards)
1:  wfe
    b 1b

.section .bss
.align 12
_stack:
    .skip 4096
_stack_top:

.section .text
.global context_switch
.type context_switch, %function
.align 4

.equ CPU_CONTEXT_X19_X20, (0 * 16)
.equ CPU_CONTEXT_X21_X22, (1 * 16)
.equ CPU_CONTEXT_X23_X24, (2 * 16)
.equ CPU_CONTEXT_X25_X26, (3 * 16)
.equ CPU_CONTEXT_X27_X28, (4 * 16)
.equ CPU_CONTEXT_FP_LR,   (5 * 16)  // x29/x30
.equ CPU_CONTEXT_SP,      (6 * 16)

// void context_switch(struct cpu_context* a, struct cpu_context* b)
context_switch:
    stp x19, x20, [x0, #CPU_CONTEXT_X19_X20]
    stp x21, x22, [x0, #CPU_CONTEXT_X21_X22]
    stp x23, x24, [x0, #CPU_CONTEXT_X23_X24]
    stp x25, x26, [x0, #CPU_CONTEXT_X25_X26]
    stp x27, x28, [x0, #CPU_CONTEXT_X27_X28]
    stp x29, x30, [x0, #CPU_CONTEXT_FP_LR]

    mov x2, sp
    str x2, [x0, #CPU_CONTEXT_SP]

    ldp x19, x20, [x1, #CPU_CONTEXT_X19_X20]
    ldp x21, x22, [x1, #CPU_CONTEXT_X21_X22]
    ldp x23, x24, [x1, #CPU_CONTEXT_X23_X24]
    ldp x25, x26, [x1, #CPU_CONTEXT_X25_X26]
    ldp x27, x28, [x1, #CPU_CONTEXT_X27_X28]
    ldp x29, x30, [x1, #CPU_CONTEXT_FP_LR]

    ldr x2, [x1, #CPU_CONTEXT_SP]
    mov sp, x2

    ret

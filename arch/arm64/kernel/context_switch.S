.section .text
.global context_switch
.type context_switch, %function
.align 4

// void context_switch(struct cpu_context* a, struct cpu_context* b)
context_switch:
    stp x19, x20, [x0, #(0 * 16)]
    stp x21, x22, [x0, #(1 * 16)]
    stp x23, x24, [x0, #(2 * 16)]
    stp x25, x26, [x0, #(3 * 16)]
    stp x27, x28, [x0, #(4 * 16)]
    stp x29, x30, [x0, #(5 * 16)]

    mov x2, sp
    str x2, [x0, #(6 * 16)]

    ldp x19, x20, [x1, #(0 * 16)]
    ldp x21, x22, [x1, #(1 * 16)]
    ldp x23, x24, [x1, #(2 * 16)]
    ldp x25, x26, [x1, #(3 * 16)]
    ldp x27, x28, [x1, #(4 * 16)]
    ldp x29, x30, [x1, #(5 * 16)]

    ldr x2, [x1, #(6 * 16)]
    mov sp, x2

    ret

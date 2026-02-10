#include "asm/context_offsets.h"

.section .text
.global context_switch
.type context_switch, %function
.align 4

context_switch:
    // x0 = prev_ctx (struct cpu_context*)
    // x1 = next_ctx (struct cpu_context*)

    // Save callee-saved registers into prev->ctx
    stp x19, x20, [x0, #CTX_X19]
    stp x21, x22, [x0, #CTX_X21]
    stp x23, x24, [x0, #CTX_X23]
    stp x25, x26, [x0, #CTX_X25]
    stp x27, x28, [x0, #CTX_X27]
    stp x29, x30, [x0, #CTX_FP]

    mov x2, sp
    str x2, [x0, #CTX_SP]

    // Restore next task
    ldp x19, x20, [x1, #CTX_X19]
    ldp x21, x22, [x1, #CTX_X21]
    ldp x23, x24, [x1, #CTX_X23]
    ldp x25, x26, [x1, #CTX_X25]
    ldp x27, x28, [x1, #CTX_X27]
    ldp x29, x30, [x1, #CTX_FP]

    ldr x2, [x1, #CTX_SP]
    mov sp, x2

    ret

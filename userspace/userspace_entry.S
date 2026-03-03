.section .text.boot
.global _start
.extern main

_start:
    bl main

    mov x8, #1 // SYS_EXIT
    svc #0

1:  wfi
    b 1b

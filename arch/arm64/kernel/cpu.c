#include "kernel/cpu.h"

extern void vectors(void);

void exception_init(void) {
    asm volatile("msr VBAR_EL1, %0" :: "r"(vectors));
    asm volatile("isb");
}

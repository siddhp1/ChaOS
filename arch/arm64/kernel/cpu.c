#include "kernel/cpu.h"

extern void vectors(void);

void exception_init(void) {
  __asm__ volatile("msr VBAR_EL1, %0" ::"r"(vectors));
  __asm__ volatile("isb");
}

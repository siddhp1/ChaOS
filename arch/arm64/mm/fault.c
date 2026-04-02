#include "fault.h"

#include <stdint.h>

#include "kernel/panic.h"
#include "kernel/printk.h"

void data_abort_handler(void) {
  uint64_t elr, esr, far, spsr;

  asm volatile("mrs %0, ELR_EL1" : "=r"(elr));
  asm volatile("mrs %0, ESR_EL1" : "=r"(esr));
  asm volatile("mrs %0, FAR_EL1" : "=r"(far));
  asm volatile("mrs %0, SPSR_EL1" : "=r"(spsr));

  printk("DATA ABORT!\nELR_EL1=%lx\nESR_EL1=%lx\nFAR_EL1=%lx\nSPSR_EL1=%lx",
         elr, esr, far, spsr);
  panic("Page fault detected");
}

void prefetch_abort_handler(void) {
  uint64_t elr, esr, far, spsr;

  asm volatile("mrs %0, ELR_EL1" : "=r"(elr));
  asm volatile("mrs %0, ESR_EL1" : "=r"(esr));
  asm volatile("mrs %0, FAR_EL1" : "=r"(far));
  asm volatile("mrs %0, SPSR_EL1" : "=r"(spsr));

  printk("PREFETCH ABORT!\nELR_EL1=%lx\nESR_EL1=%lx\nFAR_EL1=%lx\nSPSR_EL1=%lx",
         elr, esr, far, spsr);
  panic("Kernel instruction fetch fault detected");
}

#include "fault.h"

#include <stdint.h>

#include "kernel/panic.h"
#include "kernel/printk.h"

void data_abort_handler(void) {
  uintptr_t far;
  uint64_t esr;
  asm volatile("mrs %0, FAR_EL1" : "=r"(far));
  asm volatile("mrs %0, ESR_EL1" : "=r"(esr));

  printk("DATA ABORT!\n");
  panic("Page fault detected");
}

void prefetch_abort_handler(void) {
  uint64_t esr;
  uintptr_t far;

  asm volatile("mrs %0, ESR_EL1" : "=r"(esr));
  asm volatile("mrs %0, FAR_EL1" : "=r"(far));

  printk("PREFETCH ABORT!\n");
  panic("Kernel instruction fetch fault");
}

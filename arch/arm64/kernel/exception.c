#include "exception.h"

#include <stdint.h>

#include "kernel/panic.h"
#include "kernel/printk.h"

void panic_sync(void) {
  uint64_t esr, far, elr;
  asm volatile("mrs %0, ESR_EL1" : "=r"(esr));
  asm volatile("mrs %0, FAR_EL1" : "=r"(far));
  asm volatile("mrs %0, ELR_EL1" : "=r"(elr));
  printk("SYNC EXCEPTION (unhandled)\n");
  printk("  ESR_EL1 = ");
  printk_hex_u64(esr);
  printk("\n");
  printk("  FAR_EL1 = ");
  printk_hex_u64(far);
  printk("\n");
  printk("  ELR_EL1 = ");
  printk_hex_u64(elr);
  printk("\n");
  panic("Synchronous exception");
}

void panic_irq(void) { panic("IRQ exception"); }

void panic_fiq(void) { panic("FIQ exception"); }

void panic_serr(void) { panic("SError exception"); }

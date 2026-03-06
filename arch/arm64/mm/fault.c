#include "fault.h"

#include <stdint.h>

#include "kernel/panic.h"
#include "kernel/printk.h"

void data_abort_handler(void) {
  uintptr_t far;
  uint64_t esr;
  uint64_t elr;
  uint64_t sp_val;
  asm volatile("mrs %0, FAR_EL1" : "=r"(far));
  asm volatile("mrs %0, ESR_EL1" : "=r"(esr));
  asm volatile("mrs %0, ELR_EL1" : "=r"(elr));
  asm volatile("mov %0, sp" : "=r"(sp_val));

  printk("DATA ABORT!\n");
  printk("  FAR_EL1 = ");
  printk_hex_u64(far);
  printk("\n");
  printk("  ESR_EL1 = ");
  printk_hex_u64(esr);
  printk("\n");
  printk("  ELR_EL1 = ");
  printk_hex_u64(elr);
  printk("\n");
  printk("  SP      = ");
  printk_hex_u64(sp_val);
  printk("\n");
  panic("Page fault detected");
}

void prefetch_abort_handler(void) {
  uint64_t esr;
  uintptr_t far;
  uint64_t elr;

  asm volatile("mrs %0, ESR_EL1" : "=r"(esr));
  asm volatile("mrs %0, FAR_EL1" : "=r"(far));
  asm volatile("mrs %0, ELR_EL1" : "=r"(elr));

  printk("PREFETCH ABORT!\n");
  printk("  FAR_EL1 = ");
  printk_hex_u64(far);
  printk("\n");
  printk("  ESR_EL1 = ");
  printk_hex_u64(esr);
  printk("\n");
  printk("  ELR_EL1 = ");
  printk_hex_u64(elr);
  printk("\n");
  panic("Kernel instruction fetch fault");
}

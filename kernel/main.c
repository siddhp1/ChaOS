#include "kernel/printk.h"

void kernel_entry(void) {
  // Zero out the BSS (static variables)

  // Initialize exception table (call)
  // Initialize UART (call)

  printk("Hello OS!\n");

  while (1) asm volatile("WFI");  // Infinite wait for interrupt loop
}

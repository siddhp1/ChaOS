#include "kernel/cpu.h"
#include "kernel/printk.h"
#include "kernel/uart.h"

extern unsigned long bss_start;
extern unsigned long bss_end;

void kernel_entry(void) {
  // Zero out the BSS (static variables)
  unsigned long *ptr = &bss_start;
  while (ptr < &bss_end) {
    *ptr++ = 0;
  }

  uart_init();
  exception_init();

  printk("Hello OS!\n");
  // Trigger a panic to test exception handling
  volatile int *p = (int*)0xDEADBEEF;
  volatile int x = *p;
  (void)x;

  while (1) asm volatile("WFI");
}

#include <stdint.h>

#include "asm/mmu.h"
#include "kernel/cpu.h"
#include "kernel/printk.h"
#include "kernel/uart.h"

extern uintptr_t bss_start;
extern uintptr_t bss_end;

void kernel_entry(void) {
  // Zero out the BSS (static variables)
  volatile uintptr_t *ptr = (volatile uintptr_t *)&bss_start;
  while (ptr < &bss_end) {
    *ptr++ = 0;
  }

  uart_init();
  exception_init();

  printk("Hello OS!\n");

  // Trigger a panic to test exception handling
  // volatile int *p = (int *)0xDEADBEEF;
  // volatile int x = *p;
  // (void)x;

  memory_init();

  while (1) asm volatile("WFI");
}

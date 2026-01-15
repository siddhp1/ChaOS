#include "kernel/panic.h"

#include "kernel/printk.h"

void panic(const char* msg) {
  printk("PANIC: ");
  printk(msg);
  printk("\n");
  while (1) asm volatile("WFI");
}

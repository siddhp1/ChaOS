#include "kernel/printk.h"

void printk(const char* str) {
  while (*str) {
    // uart_putc(*str++);
  }
}

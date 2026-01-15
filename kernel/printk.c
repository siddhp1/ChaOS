#include "kernel/printk.h"

#include "kernel/uart.h"

void printk(const char* str) {
  while (*str) {
    uart_putc(*str++);
  }
}

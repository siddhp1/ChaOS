#include "kernel/printk.h"

#include <stdint.h>

#include "kernel/uart.h"

void printk_hex_u32(uint32_t num) {
  char buffer[11];  // 2 + 8 + 1
  buffer[0] = '0';
  buffer[1] = 'x';
  for (int i = 0; i < 8; i++) {
    uint32_t nibble = (num >> (28 - 4 * i)) & 0xF;
    buffer[2 + i] = (nibble < 10) ? ('0' + nibble) : ('A' + (nibble - 10));
  }
  buffer[10] = '\0';
  printk(buffer);
}

void printk_hex_u64(uint64_t num) {
  char buffer[19];  // 2 + 16 + 1
  buffer[0] = '0';
  buffer[1] = 'x';
  for (int i = 0; i < 16; i++) {
    uint64_t nibble = (num >> (60 - 4 * i)) & 0xF;
    buffer[2 + i] = (nibble < 10) ? ('0' + nibble) : ('A' + (nibble - 10));
  }
  buffer[18] = '\0';
  printk(buffer);
}

void printk(const char* str) {
  while (*str) {
    uart_putc(*str++);
  }
}

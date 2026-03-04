#include "kernel/uart.h"

#define UART0_BASE 0x09000000UL
#define UART_DR (*(volatile unsigned int*)(UART0_BASE + 0x00))
#define UART_FR (*(volatile unsigned int*)(UART0_BASE + 0x18))

#define TXFF (1 << 5)
#define RXFE (1 << 4)

void uart_init(void) {}

void uart_putc(char c) {
  while (UART_FR & TXFF);
  UART_DR = c;
}

char uart_getc(void) {
  while (UART_FR & RXFE);
  return (char)(UART_DR & 0xFF);
}

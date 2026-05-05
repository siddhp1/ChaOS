#include "drivers/uart.h"

#include "mm/kmap.h"

#define MMIO_BASE (KERNEL_VIRT_BASE + 0x3F000000UL)
#define GPFSEL1 (*(volatile unsigned int*)(MMIO_BASE + 0x200004))
#define GPPUD (*(volatile unsigned int*)(MMIO_BASE + 0x200094))
#define GPPUDCLK0 (*(volatile unsigned int*)(MMIO_BASE + 0x200098))

#define UART0_BASE (MMIO_BASE + 0x201000)
#define UART_DR (*(volatile unsigned int*)(UART0_BASE + 0x00))
#define UART_FR (*(volatile unsigned int*)(UART0_BASE + 0x18))
#define UART_IBRD (*(volatile unsigned int*)(UART0_BASE + 0x24))
#define UART_FBRD (*(volatile unsigned int*)(UART0_BASE + 0x28))
#define UART_LCRH (*(volatile unsigned int*)(UART0_BASE + 0x2C))
#define UART_CR (*(volatile unsigned int*)(UART0_BASE + 0x30))

#define RXFE (1 << 4)
#define TXFF (1 << 5)

static void delay(int count) {
  while (count--) {
    asm volatile("nop");
  }
}

void uart_init(void) {
  // Disable UART
  UART_CR = 0;

  // Set GPIO14/15 to ALT0 (TXD0/RXD0)
  unsigned int r = GPFSEL1;
  r &= ~((7u << 12) | (7u << 15));
  r |= ((4u << 12) | (4u << 15));
  GPFSEL1 = r;

  // Disable pulls for GPIO14/15
  GPPUD = 0;
  delay(150);
  GPPUDCLK0 = (1u << 14) | (1u << 15);
  delay(150);
  GPPUDCLK0 = 0;

  // Set baud rate (115200)
  UART_IBRD = 26;
  UART_FBRD = 3;

  // 8N1 + FIFO enabled
  UART_LCRH = (1u << 4) | (1u << 5) | (1u << 6);

  // Enable UART, TX, RX
  UART_CR = (1u << 0) | (1u << 8) | (1u << 9);  // UARTEN | TXE | RXE
}

char uart_getc(void) {
  while (UART_FR & RXFE);
  return (char)(UART_DR & 0xFF);
}

void uart_putc(char c) {
  if (c == '\n') uart_putc('\r');  // For serial terminals
  while (UART_FR & TXFF);
  UART_DR = c;
}

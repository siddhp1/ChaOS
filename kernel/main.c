#define MMIO_BASE 0x3F000000UL

#define GPFSEL1 ((volatile unsigned int *)(MMIO_BASE + 0x200004))
#define GPPUD ((volatile unsigned int *)(MMIO_BASE + 0x200094))
#define GPPUDCLK0 ((volatile unsigned int *)(MMIO_BASE + 0x200098))

#define UART0_BASE (MMIO_BASE + 0x201000)
#define UART0_DR ((volatile unsigned int *)(UART0_BASE + 0x00))
#define UART0_FR ((volatile unsigned int *)(UART0_BASE + 0x18))
#define UART0_IBRD ((volatile unsigned int *)(UART0_BASE + 0x24))
#define UART0_FBRD ((volatile unsigned int *)(UART0_BASE + 0x28))
#define UART0_LCRH ((volatile unsigned int *)(UART0_BASE + 0x2C))
#define UART0_CR ((volatile unsigned int *)(UART0_BASE + 0x30))

#define TXFF (1 << 5)

static void delay(int count) {
  while (count--) {
    asm volatile("nop");
  }
}

static void uart_init(void) {
  *UART0_CR = 0; // Disable UART0

  // Set GPIO14/15 to ALT0 (TXD0/RXD0)
  unsigned int r = *GPFSEL1;
  r &= ~((7u << 12) | (7u << 15));
  r |= ((4u << 12) | (4u << 15));
  *GPFSEL1 = r;

  // Disable pulls for GPIO14/15
  *GPPUD = 0;
  delay(150);
  *GPPUDCLK0 = (1u << 14) | (1u << 15);
  delay(150);
  *GPPUDCLK0 = 0;

  // Set baud rate (115200)
  *UART0_IBRD = 26;
  *UART0_FBRD = 3;

  // 8N1 + FIFO enabled
  *UART0_LCRH = (1u << 4) | (1u << 5) | (1u << 6);

  // Enable UART, TX, RX
  *UART0_CR = (1u << 0) | (1u << 8) | (1u << 9); // UARTEN | TXE | RXE
}

static void uart_putc(char c) {
  if (c == '\n')
    uart_putc('\r');
  while (*UART0_FR & TXFF)
    ;
  *UART0_DR = (unsigned int)c;
}

static void uart_puts(const char *s) {
  while (*s)
    uart_putc(*s++);
}

void kernel_main(void) {
  uart_init();
  uart_puts("Hello World!\n");
  while (1) {
  }
}

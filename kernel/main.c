#define UART0_BASE 0x09000000
#define UARTDR ((volatile unsigned int *)(UART0_BASE + 0x00))

static void uart_putc(char c) {
    *UARTDR = c;
}

static void uart_puts(const char *s) {
    while (*s) {
        uart_putc(*s++);
    }
}

void kernel_main(void) {
    uart_puts("Hello World!");
    while (1) {}
}

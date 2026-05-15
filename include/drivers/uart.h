#ifndef UART_H
#define UART_H

void uart_init(void);
void uart_irq_init(void);

void uart_putc(char c);
char uart_getc(void);
long uart_read(char* dst, long len);
long uart_write(const char* src, long len);

#endif

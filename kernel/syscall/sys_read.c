#include "kernel/uart.h"
#include "kernel/user_access.h"

long sys_read(long fd, long buffer, long length, long a3, long a4, long a5) {
  (void)a3;
  (void)a4;
  (void)a5;

  if (fd != 0) return -1;
  if (!user_range_ok(buffer, length)) return -1;

  char* buf = (char*)buffer;
  long i = 0;

  while (i < length) {
    char c = uart_getc();

    if (c == '\r' || c == '\n') {
      uart_putc('\n');
      break;
    } else if (c == '\b' || c == 127) {
      if (i > 0) {
        i--;
        uart_putc('\b');
        uart_putc(' ');
        uart_putc('\b');
      }
    } else {
      buf[i++] = c;
      uart_putc(c);
    }
  }
  return i;
}

#include "drivers/uart.h"
#include "kernel/user_access.h"

long sys_read(long fd, long buffer, long length, long a3, long a4, long a5) {
  (void)a3;
  (void)a4;
  (void)a5;

  if (fd != 0) return -1;
  if (length < 0) return -1;
  if (length == 0) return 0;
  if (!user_range_ok((uintptr_t)buffer, (uint64_t)length)) return -1;

  char* buf = (char*)buffer;
  long i = 0;

  while (i < length) {
    char c;
    if (uart_read(&c, 1) != 1) break;

    if (c == '\r' || c == '\n') {
      char nl = '\n';
      if (copy_to_user(buf + i, &nl, 1) < 0) return -1;
      uart_write("\n", 1);
      i++;
      break;
    }

    if (c == '\b' || c == 127) {
      if (i > 0) {
        i--;
        uart_write("\b \b", 3);
      }
      continue;
    }

    if (copy_to_user(buf + i, &c, 1) < 0) return -1;
    uart_write(&c, 1);
    i++;
  }

  return i;
}

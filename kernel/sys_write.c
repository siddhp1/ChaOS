#include <stddef.h>

#include "kernel/uart.h"

long sys_write(long file_descriptor, long buffer, long length, long a3, long a4,
               long a5) {
  const char* user_buffer = (const char*)buffer;

  // Only stdout supported
  if (file_descriptor != 1) {
    return -1;
  }

  if (length < 0) {
    return -1;
  }

  if (length == 0) {
    return 0;
  }

  for (size_t i = 0; i < length; i++) {
    uart_putc(user_buffer[i]);
  }

  return length;
}

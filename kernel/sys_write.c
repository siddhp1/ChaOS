#include "kernel/uart.h"

long sys_write(long file_descriptor, long buffer, long length, long a3, long a4,
               long a5) {
  // Only stdout supported
  if (file_descriptor != 1) {
    return -1;
  }

  char* user_buffer = (char*)buffer;

  for (long i = 0; i < length; i++) {
    uart_putc(user_buffer[i]);
  }

  return length;
}

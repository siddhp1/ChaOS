#include "kernel/uaccess.h"
#include "kernel/uart.h"

long sys_write(long file_descriptor, long buffer, long length, long a3, long a4,
               long a5) {
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

  const char* user_buffer = (const char*)buffer;
  long written = 0;

  // Kernel staging buffer to avoid direct EL1 reads from user pointer
  char kbuf[128];

  while (written < length) {
    long remaining = length - written;
    long chunk =
        (remaining > (long)sizeof(kbuf)) ? (long)sizeof(kbuf) : remaining;

    // TODO: Fix side-effect pattern here
    if (copy_from_user(kbuf, user_buffer + written, (uint64_t)chunk) < 0) {
      return -1;
    }

    for (long i = 0; i < chunk; i++) {
      uart_putc(kbuf[i]);
    }

    written += chunk;
  }

  return written;
}

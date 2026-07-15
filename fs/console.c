#include "fs/console.h"

#include "drivers/uart.h"
#include "fs/file.h"

// long console_read(struct file* file, void* buf, size_t len) {
//   return uart_write((const char*)buf, len);
// }

long console_write(struct file* file, const void* buf, size_t len) {
  return uart_write((const char*)buf, len);
}

static const struct file_ops console_ops = {
    .read = NULL, .write = console_write, .close = NULL};

struct file console_file = {.refcount = 1,
                            .flags = 0,
                            .position = 0,
                            .ops = &console_ops,
                            .data = NULL};

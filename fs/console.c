#include "fs/console.h"

#include <stddef.h>

#include "drivers/uart.h"
#include "fs/file.h"

/**
 * @file
 * @brief UART-backed console files and process standard-I/O setup.
 */

/**
 * @brief Operations table shared by all console files.
 */
static const struct file_ops console_ops = {
    .read = console_read, .write = console_write, .release = NULL};

struct file* console_file_open(int flags) {
  return file_alloc(&console_ops, (uint32_t)flags, NULL);
}

long console_read(struct file* file, void* buf, size_t len) {
  (void)file;

  char* kbuf = (char*)buf;
  size_t count = 0;

  if (len == 0) return 0;
  if (!buf) return -1;

  while (count < len) {
    char c;

    if (uart_read(&c, 1) != 1) break;

    if (c == '\r' || c == '\n') {
      kbuf[count++] = '\n';
      uart_write("\n", 1);
      break;
    }

    // Remove the previous input character and erase its echoed character
    if (c == '\b' || c == 127) {
      if (count > 0) {
        count--;
        uart_write("\b \b", 3);
      }
      continue;
    }

    kbuf[count++] = c;

    uart_write(&c, 1);
  }

  return (long)count;
}

long console_write(struct file* file, const void* buf, size_t len) {
  (void)file;

  if (len == 0) return 0;
  if (!buf) return -1;

  return uart_write((const char*)buf, len);
}

int process_stdio_init(struct task* task) {
  if (!task) return -1;

  for (int fd = 0; fd < 3; ++fd) {
    if (fd_get(task, fd)) return -1;
  }

  int installed = 0;

  for (int fd = 0; fd < 3; ++fd) {
    struct file* file = console_file_open(0);
    if (!file) goto fail;

    if (fd_install_at(task, file, fd) < 0) {
      file_unref(file);
      goto fail;
    }
    installed++;
  }

  return 0;

fail:
  while (installed > 0) fd_close(task, --installed);
  return -1;
}

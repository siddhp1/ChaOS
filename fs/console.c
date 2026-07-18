#include "fs/console.h"

#include "drivers/uart.h"
#include "fs/file.h"

static const struct file_ops console_ops = {
    .read = console_read, .write = console_write, .release = NULL};

static struct file console_file = {.refcount = 1,
                                   .flags = 0,
                                   .position = 0,
                                   .f_ops = &console_ops,
                                   .data = NULL};

// TODO: Consider removing
struct file* console_file_open(int flags) {
  return file_alloc(&console_ops, flags, &console_file);
}

long console_read(struct file* file, void* buf, size_t len) {
  (void)file;

  char* kbuf = (char*)buf;
  long count = 0;

  if (!buf || len < 0) return -1;
  if (len == 0) return 0;

  while (count < len) {
    char c;

    if (uart_read(&c, 1) != 1) break;

    if (c == '\r' || c == '\n') {
      kbuf[count++] = '\n';
      uart_write("\n", 1);
      break;
    }

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

  return count;
}

long console_write(struct file* file, const void* buf, size_t len) {
  (void)file;

  if (!buf || len < 0) return -1;
  if (len == 0) return 0;

  return uart_write((const char*)buf, len);
}

int process_stdio_init(struct task* task) {
  if (!task) {
    return -1;
  }

  struct file* file;

  file = file_alloc(&console_ops, 0, &console_file);
  if (!file) {
    goto fail;
  }

  if (fd_install_at(task, file, 0) < 0) {
    file_unref(file);
    goto fail;
  }

  file = file_alloc(&console_ops, 0, &console_file);
  if (!file) {
    goto fail;
  }

  if (fd_install_at(task, file, 1) < 0) {
    file_unref(file);
    goto fail;
  }

  file = file_alloc(&console_ops, 0, &console_file);
  if (!file) {
    goto fail;
  }

  if (fd_install_at(task, file, 2) < 0) {
    file_unref(file);
    goto fail;
  }

  return 0;

fail:
  fd_table_close_all(task);
  return -1;
}

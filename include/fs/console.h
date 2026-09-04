#ifndef CONSOLE_H
#define CONSOLE_H

#include "fs/file.h"

long console_read(struct file* file, void* buf, size_t len);
long console_write(struct file* file, const void* buf, size_t len);

struct file* console_file_open(int flags);
int process_stdio_init(struct task* task);

#endif

#ifndef CONSOLE_H
#define CONSOLE_H

#include "fs/file.h"

long console_read(struct file* file, void* buf, size_t len);
long console_write(struct file* file, const void* buf, size_t len);

extern struct file console_file;

#endif

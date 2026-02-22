#ifndef LIBC_H
#define LIBC_H

static inline long syscall(long nr, long a0, long a1, long a2, long a3, long a4,
                           long a5);
long write(int file_descriptor, const void* buffer, long length);
void exit(int status);

#endif

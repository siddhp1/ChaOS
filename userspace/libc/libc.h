#ifndef LIBC_H
#define LIBC_H

#define SYS_WRITE 0
#define SYS_EXIT 1
#define SYS_EXECVE 3
#define SYS_FORK 4
#define SYS_READ 5

long syscall(long nr, long a0, long a1, long a2, long a3, long a4, long a5);

long write(int fd, const void* buf, long len);
int execve(const char* path, char* const argv[]);
void exit(int status);
int fork(void);
long read(int fd, void* buf, long len);

#endif

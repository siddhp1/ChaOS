#ifndef LIBC_H
#define LIBC_H

#define SYS_WRITE 0
#define SYS_EXIT 1
#define SYS_EXECVE 3
#define SYS_FORK 4
#define SYS_READ 5
#define SYS_WAIT 6
#define SYS_WAITPID 7
#define SYS_OPEN 8
#define SYS_CLOSE 9
#define SYS_DUP 10
#define SYS_DUP2 11

long syscall(long nr, long a0, long a1, long a2, long a3, long a4, long a5);

long write(int fd, const void* buf, long len);
long execve(const char* path, char* const argv[]);
void exit(int status);
long fork(void);
long read(int fd, void* buf, long len);
long wait(int* status);
long waitpid(int pid, int* status);
long open(const char* user_path_addr, int flags, int mode);
long close(int fd);
long dup(int old_fd);
long dup2(int old_fd, int new_fd);

#endif

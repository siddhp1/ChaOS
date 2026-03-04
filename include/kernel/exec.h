#ifndef EXEC_H
#define EXEC_H

long sys_execve(long filepath, long a1, long a2, long a3, long a4, long a5);
void load_init(void);

#endif

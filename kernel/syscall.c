#include "kernel/syscall.h"

#include "kernel/trap.h"
#include "syscall_handlers.h"

static syscall_fn_t syscall_table[SYS_MAX] = {
    [SYS_WRITE] = sys_write,
    [SYS_EXIT] = sys_exit,
    [SYS_GETPID] = sys_getpid,
};

long syscall_dispatch(long nr, long a0, long a1, long a2, long a3, long a4,
                      long a5) {
  if (nr < 0 || nr >= SYS_MAX) {
    return -1;
  }

  syscall_fn_t fn = syscall_table[nr];
  if (!fn) {
    return -1;
  }

  return fn(a0, a1, a2, a3, a4, a5);
}

void syscall_entry(void* frame) {
  struct trapframe* tf = (struct trapframe*)frame;

  long nr = tf->x[8];
  long a0 = tf->x[0];
  long a1 = tf->x[1];
  long a2 = tf->x[2];
  long a3 = tf->x[3];
  long a4 = tf->x[4];
  long a5 = tf->x[5];

  long ret = syscall_dispatch(nr, a0, a1, a2, a3, a4, a5);

  tf->x[0] = ret;
}

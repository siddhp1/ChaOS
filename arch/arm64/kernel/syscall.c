#include "syscall.h"

#include "trap.h"

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

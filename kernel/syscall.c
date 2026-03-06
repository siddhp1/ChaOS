#include "kernel/syscall.h"

#include "kernel/exec.h"
#include "kernel/fork.h"
#include "kernel/printk.h"
#include "kernel/scheduler.h"
#include "kernel/string.h"
#include "kernel/trap.h"
#include "syscall_handlers.h"

static syscall_fn_t syscall_table[SYS_MAX] = {
    [SYS_WRITE] = sys_write,   [SYS_EXIT] = sys_exit, [SYS_GETPID] = sys_getpid,
    [SYS_EXECVE] = sys_execve, [SYS_FORK] = sys_fork, [SYS_READ] = sys_read,
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

  // Store frame pointer for sys_execve and sys_fork
  current_task->irq_sp = (uint64_t)tf;

  long nr = tf->x[8];
  long a0 = tf->x[0];
  long a1 = tf->x[1];
  long a2 = tf->x[2];
  long a3 = tf->x[3];
  long a4 = tf->x[4];
  long a5 = tf->x[5];

  if (nr == SYS_WRITE) {
    printk("PID: ");
    printk_hex_u32(current_task->pid);
  }

  long ret = syscall_dispatch(nr, a0, a1, a2, a3, a4, a5);

  // After successful execve the address space was replaced.
  // Rewrite the trapframe so eret enters the new program instead of
  // returning to the (now-gone) caller.
  if (nr == SYS_EXECVE && ret == 0) {
    memset(tf, 0, sizeof(*tf));
    tf->elr_el1 = current_task->user_entry;
    tf->sp_el0 = current_task->user_sp;
    tf->spsr_el1 = 0;  // EL0t
    return;
  }

  tf->x[0] = ret;
}

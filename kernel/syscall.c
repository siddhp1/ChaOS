#include "kernel/syscall.h"

#include <stddef.h>
#include <stdint.h>

#include "kernel/printk.h"
#include "kernel/scheduler.h"
#include "kernel/trap.h"
#include "kernel/uart.h"
#include "syscall_handlers.h"

static syscall_fn_t syscall_table[SYS_MAX] = {
    [SYS_WRITE] = sys_write, [SYS_EXIT] = sys_exit, [SYS_EXECVE] = sys_execve,
    [SYS_FORK] = sys_fork,   [SYS_READ] = sys_read,
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

void handle_el0_sync(void* frame) {
  struct trapframe* tf = (struct trapframe*)frame;

  // Ensure irq sp is not stale
  if (current_task) {
    current_task->irq_sp = (uint64_t)frame;
  }

  uint64_t esr;
  asm volatile("mrs %0, ESR_EL1" : "=r"(esr));

  uint32_t exception = (esr >> 26) & 0x3F;  // Exception class

  if (exception == 0x15) {  // SVC instruction
    long nr = tf->x[8];
    long a0 = tf->x[0];
    long a1 = tf->x[1];
    long a2 = tf->x[2];
    long a3 = tf->x[3];
    long a4 = tf->x[4];
    long a5 = tf->x[5];

    long ret = syscall_dispatch(nr, a0, a1, a2, a3, a4, a5);

    tf->x[0] = ret;
  } else {
    printk("EL0 sync exception, EC=%u, ESR=%lu\n", exception, esr);

    // TODO: Switch to panic
    while (1) {
      asm volatile("wfi");
    }
  }
}

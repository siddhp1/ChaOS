#include <stddef.h>

#include "kernel/initramfs.h"
#include "kernel/irq.h"
#include "kernel/printk.h"
#include "kernel/scheduler/scheduler.h"
#include "kernel/task.h"
#include "mm/mmu.h"
#include "mm/pgtable.h"
#include "syscall_handlers.h"

long sys_exit(long status, long a1, long a2, long a3, long a4, long a5) {
  (void)a1;
  (void)a2;
  (void)a3;
  (void)a4;
  (void)a5;

  task_exit(current_task, (int32_t)status);

  return 0;
}

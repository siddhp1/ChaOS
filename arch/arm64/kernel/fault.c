#include <stdint.h>

#include "kernel/printk.h"
#include "kernel/scheduler.h"
#include "kernel/task.h"

void el0_fault_handler(void* frame, uint64_t esr) {
  (void)frame;

  uint64_t far = 0;
  asm volatile("mrs %0, FAR_EL1" : "=r"(far));

  // TODO: Switch to updated printk
  printk("EL0 FAULT: pid=");
  if (current_task) {
    printk_hex_u32((uint32_t)current_task->pid);
  } else {
    printk("0");
  }
  printk(" ec=");
  printk_hex_u64((esr >> 26) & 0x3f);
  printk(" far=");
  printk_hex_u64(far);
  printk("\n");

  if (current_task) {
    current_task->state = TASK_ZOMBIE;
  }
  need_schedule = true;
}

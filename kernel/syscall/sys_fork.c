#include <stddef.h>
#include <stdint.h>

#include "kernel/irq_frame.h"
#include "kernel/pid.h"
#include "kernel/printk.h"
#include "kernel/scheduler/scheduler.h"
#include "kernel/string.h"
#include "kernel/task.h"
#include "mm/kmap.h"
#include "mm/page.h"
#include "mm/pgtable.h"
#include "mm/user_pgtable.h"
#include "syscall_handlers.h"

long sys_fork(long a0, long a1, long a2, long a3, long a4, long a5) {
  (void)a0;
  (void)a1;
  (void)a2;
  (void)a3;
  (void)a4;
  (void)a5;

  struct task* parent = current_task;

  struct task* child = alloc_task();
  if (!child) {
    printk("sys_fork: Failed to allocate child task\n");
    return -1;
  }

  child->mode = TASK_MODE_USER;
  child->pid = pid_alloc();

  struct page* parent_pgd_page = phys_to_page(parent->ttbr0);
  if (!parent_pgd_page) {
    printk("sys_fork: Invalid parent TTBR0\n");
    free_page((struct page*)child);
    return -1;
  }

  uint64_t* parent_pgd_va = (uint64_t*)kmap(parent_pgd_page);
  uintptr_t child_pgd_va = copy_user_pgd(parent_pgd_va);
  if (!child_pgd_va) {
    printk("sys_fork: Failed to copy page tables\n");
    free_page((struct page*)child);
    return -1;
  }

  child->ttbr0 = kernel_to_phys(child_pgd_va);

  void* child_kstack = alloc_stack();
  if (!child_kstack) {
    printk("sys_fork: Failed to allocate kernel stack\n");
    destroy_task(child);
    return -1;
  }

  child->stack = (uint64_t)child_kstack;

  struct irq_frame* parent_frame = (struct irq_frame*)parent->irq_sp;
  uint64_t child_frame_addr =
      ((uint64_t)child_kstack + PAGE_SIZE) - IRQ_FRAME_SIZE;

  struct irq_frame* child_frame = (struct irq_frame*)child_frame_addr;
  memcpy(child_frame, parent_frame, IRQ_FRAME_SIZE);

  // Child returns 0 from fork
  child_frame->x[0] = 0;

  child->irq_sp = child_frame_addr;

  child->time_slice = DEFAULT_TIME_SLICE;

  add_child(parent, child);

  printk("sys_fork: Created child PID=%d\n", child->pid);

  enqueue_task(child);
  return child->pid;
}

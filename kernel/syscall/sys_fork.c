#include <stddef.h>
#include <stdint.h>

#include "kernel/pid.h"
#include "kernel/printk.h"
#include "kernel/process.h"
#include "kernel/scheduler/scheduler.h"
#include "kernel/string.h"
#include "kernel/task.h"
#include "mm/kmap.h"
#include "mm/page.h"
#include "mm/pgtable.h"
#include "mm/user_pgtable.h"
#include "syscall_handlers.h"

// TODO: Sync with vectors
#define IRQ_FRAME_SIZE 288

struct irq_frame {
  uint64_t regs[31];
  uint64_t elr_el1;
  uint64_t spsr_el1;
  uint64_t sp_el0;
  uint64_t padding;
};

extern void fork_child_return(void);

long sys_fork(long a0, long a1, long a2, long a3, long a4, long a5) {
  (void)a0;
  (void)a1;
  (void)a2;
  (void)a3;
  (void)a4;
  (void)a5;

  printk("sys_fork: called by PID=%d\n", current_task->pid);

  struct task* parent = current_task;

  struct task* child = alloc_task();
  if (!child) {
    printk("sys_fork: Failed to allocate child task\n");
    return -1;
  }

  printk("sys_fork: Copying page tables\n");
  printk("Parent TTBR0 (phys): %lx\n", parent->ttbr0);

  struct page* parent_pgd_page = phys_to_page(parent->ttbr0);
  if (!parent_pgd_page) {
    printk("sys_fork: Invalid parent TTBR0\n");
    uint64_t task_va = (uint64_t)child;
    uint64_t task_phys = task_va - KERNEL_VIRT_BASE;
    struct page* task_page = phys_to_page(task_phys);
    if (task_page) free_page(task_page);
    return -1;
  }

  uint64_t* parent_pgd_va = (uint64_t*)kmap(parent_pgd_page);
  printk("Parent PGD (virt): %lx\n", (uint64_t)parent_pgd_va);

  uint64_t* child_pgd_va = copy_user_pgd(parent_pgd_va);
  if (!child_pgd_va) {
    printk("sys_fork: Failed to copy page tables\n");
    uint64_t task_va = (uint64_t)child;
    uint64_t task_phys = task_va - KERNEL_VIRT_BASE;
    struct page* task_page = phys_to_page(task_phys);
    if (task_page) free_page(task_page);
    return -1;
  }

  uint64_t child_pgd_phys = ((uint64_t)child_pgd_va) - KERNEL_VIRT_BASE;
  printk("Child PGD (virt): %lx\n", (uint64_t)child_pgd_va);
  printk("Child TTBR0 (phys): %lx\n", child_pgd_phys);

  child->ttbr0 = child_pgd_phys;
  child->mode = TASK_MODE_USER;
  child->sp_el0 = parent->sp_el0;

  printk("sys_fork: Allocating kernel stack\n");
  void* child_kstack = alloc_stack();
  if (!child_kstack) {
    printk("sys_fork: Failed to allocate kernel stack\n");
    free_user_pgd(child_pgd_va);
    uint64_t task_va = (uint64_t)child;
    uint64_t task_phys = task_va - KERNEL_VIRT_BASE;
    struct page* task_page = phys_to_page(task_phys);
    if (task_page) free_page(task_page);
    return -1;
  }

  child->stack = (uint64_t)child_kstack;

  printk("sys_fork: Copying IRQ frame from parent\n");
  printk("Parent irq_sp: %lx\n", parent->irq_sp);

  struct irq_frame* parent_frame = (struct irq_frame*)parent->irq_sp;

  // Child's IRQ frame will be at the top of its kernel stack
  uint64_t child_frame_addr =
      ((uint64_t)child_kstack + PAGE_SIZE) - IRQ_FRAME_SIZE;
  struct irq_frame* child_frame = (struct irq_frame*)child_frame_addr;

  memcpy(child_frame, parent_frame, IRQ_FRAME_SIZE);

  // Child returns 0 from fork
  child_frame->regs[0] = 0;

  printk("Child frame at: %lx\n", child_frame_addr);
  printk("Child x0 (return value): %lx\n", child_frame->regs[0]);

  child->context.sp = child_frame_addr;
  child->context.lr = (uint64_t)fork_child_return;

  // TODO: Switch to memcpy
  child->context.x19 = parent->context.x19;
  child->context.x20 = parent->context.x20;
  child->context.x21 = parent->context.x21;
  child->context.x22 = parent->context.x22;
  child->context.x23 = parent->context.x23;
  child->context.x24 = parent->context.x24;
  child->context.x25 = parent->context.x25;
  child->context.x26 = parent->context.x26;
  child->context.x27 = parent->context.x27;
  child->context.x28 = parent->context.x28;
  child->context.fp = parent->context.fp;

  child->irq_sp = child_frame_addr;

  child->pid = pid_alloc();
  child->state = TASK_READY;
  child->time_slice = DEFAULT_TIME_SLICE;
  child->exit_status = 0;

  add_child(parent, child);

  printk("sys_fork: Created child PID=%d\n", child->pid);

  enqueue_task(child);
  return child->pid;
}

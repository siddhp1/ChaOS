#include <stddef.h>
#include <stdint.h>

#include "kernel/printk.h"
#include "kernel/process.h"
#include "kernel/scheduler.h"
#include "kernel/string.h"
#include "kernel/task.h"
#include "mm/kmap.h"
#include "mm/page.h"
#include "mm/pgtable.h"
#include "mm/user_pgtable.h"
#include "pid.h"
#include "syscall_handlers.h"
#include "task_internal.h"

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

  printk("sys_fork: called by PID=");
  printk_hex_u64(current_task->pid);
  printk("\n");

  struct task* parent = current_task;

  struct task* child = alloc_task();
  if (!child) {
    printk("sys_fork: Failed to allocate child task\n");
    return -1;
  }

  printk("sys_fork: Copying page tables\n");
  printk("  Parent TTBR0 (phys): ");
  printk_hex_u64(parent->ttbr0);
  printk("\n");

  struct page* parent_pgd_page = phys_to_page(parent->ttbr0);
  if (!parent_pgd_page) {
    printk("sys_fork: Invalid parent TTBR0\n");
    uint64_t task_va = (uint64_t)child;
    uint64_t task_phys = task_va - KERNEL_BASE;
    struct page* task_page = phys_to_page(task_phys);
    if (task_page) free_page(task_page);
    return -1;
  }

  uint64_t* parent_pgd_va = (uint64_t*)kmap(parent_pgd_page);
  printk("  Parent PGD (virt): ");
  printk_hex_u64((uint64_t)parent_pgd_va);
  printk("\n");

  uint64_t* child_pgd_va = copy_user_pgd(parent_pgd_va);
  if (!child_pgd_va) {
    printk("sys_fork: Failed to copy page tables\n");
    uint64_t task_va = (uint64_t)child;
    uint64_t task_phys = task_va - KERNEL_BASE;
    struct page* task_page = phys_to_page(task_phys);
    if (task_page) free_page(task_page);
    return -1;
  }

  uint64_t child_pgd_phys = ((uint64_t)child_pgd_va) - KERNEL_BASE;
  printk("  Child PGD (virt): ");
  printk_hex_u64((uint64_t)child_pgd_va);
  printk("\n");
  printk("  Child TTBR0 (phys): ");
  printk_hex_u64(child_pgd_phys);
  printk("\n");

  child->ttbr0 = child_pgd_phys;
  child->mode = TASK_MODE_USER;
  child->sp_el0 = parent->sp_el0;

  printk("sys_fork: Allocating kernel stack\n");
  void* child_kstack = alloc_stack();
  if (!child_kstack) {
    printk("sys_fork: Failed to allocate kernel stack\n");
    free_user_pgd(child_pgd_va);
    uint64_t task_va = (uint64_t)child;
    uint64_t task_phys = task_va - KERNEL_BASE;
    struct page* task_page = phys_to_page(task_phys);
    if (task_page) free_page(task_page);
    return -1;
  }

  child->stack = (uint64_t)child_kstack;

  printk("sys_fork: Copying IRQ frame from parent\n");
  printk("  Parent irq_sp: ");
  printk_hex_u64(parent->irq_sp);
  printk("\n");

  struct irq_frame* parent_frame = (struct irq_frame*)parent->irq_sp;

  // Child's IRQ frame will be at the top of its kernel stack
  uint64_t child_frame_addr =
      ((uint64_t)child_kstack + PAGE_SIZE) - IRQ_FRAME_SIZE;
  struct irq_frame* child_frame = (struct irq_frame*)child_frame_addr;

  memcpy(child_frame, parent_frame, IRQ_FRAME_SIZE);

  // Child returns 0 from fork
  child_frame->regs[0] = 0;

  printk("  Child frame at: ");
  printk_hex_u64(child_frame_addr);
  printk("\n");
  printk("  Child x0 (return value): ");
  printk_hex_u64(child_frame->regs[0]);
  printk("\n");

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

  printk("sys_fork: Created child PID=");
  printk_hex_u64(child->pid);
  printk("\n");

  enqueue_task(child);
  return child->pid;
}

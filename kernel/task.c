#include "kernel/task.h"

#include <stddef.h>

#include "kernel/irq.h"
#include "kernel/irq_frame.h"
#include "kernel/printk.h"
#include "kernel/string.h"
#include "mm/kmap.h"
#include "mm/page.h"
#include "mm/user_pgtable.h"

void* alloc_stack(void) {
  struct page* p = alloc_page();
  if (!p) {
    return NULL;
  }

  void* va = kmap(p);
  return va;  // Bottom of stack page
}

struct task* alloc_task(void) {
  struct page* p = alloc_page();
  if (!p) {
    return NULL;
  }

  struct task* t = (struct task*)kmap(p);

  memset(t, 0, sizeof(*t));

  return t;
}

void create_irq_frame(struct task* task, uintptr_t stack_top,
                      uintptr_t entry_fn, uintptr_t user_stack_top) {
  uint64_t frame_sp = stack_top - IRQ_FRAME_SIZE;
  memset((void*)frame_sp, 0, IRQ_FRAME_SIZE);

  volatile uint64_t* elr_spsr =
      (volatile uint64_t*)(frame_sp + IRQ_OFF_ELR_SPSR);

  elr_spsr[0] = entry_fn;
  elr_spsr[1] = SPSR_EL1H;

  volatile uint64_t* sp_el0 = (volatile uint64_t*)(frame_sp + IRQ_OFF_USER_SP);
  *sp_el0 = user_stack_top;

  task->irq_sp = frame_sp;
}

void destroy_task(struct task* task) {
  if (!task) {
    return;
  }

  printk("Destroying task PID=%d\n", task->pid);

  if (task->parent) {
    remove_child(task->parent, task);
  }

  if (task->first_child) {
    struct task* child = task->first_child;
    while (child) {
      child->parent = NULL;
      child = child->sibling_next;
    }
  }

  if (task->mode == TASK_MODE_USER && task->ttbr0) {
    free_user_pgd(task->ttbr0);
    task->ttbr0 = 0;
  }

  if (task->stack) {
    uint64_t stack_va = task->stack;
    struct page* stack_page = phys_to_page(kernel_to_phys(stack_va));
    if (stack_page) {
      free_page(stack_page);
    }
    task->stack = 0;
  }

  uint64_t task_va = (uint64_t)task;
  uint64_t task_phys = task_va - KERNEL_VIRT_BASE;
  struct page* task_page = phys_to_page(task_phys);
  if (task_page) {
    free_page(task_page);
  }
}

void add_child(struct task* parent, struct task* child) {
  if (!parent || !child) return;

  irq_disable();

  child->parent = parent;
  child->sibling_next = parent->first_child;
  parent->first_child = child;

  irq_enable();
}

void remove_child(struct task* parent, struct task* child) {
  if (!parent || !child) return;

  irq_disable();

  if (parent->first_child == child) {
    parent->first_child = child->sibling_next;
  } else {
    struct task* sibling = parent->first_child;
    while (sibling && sibling->sibling_next != child) {
      sibling = sibling->sibling_next;
    }
    if (sibling) {
      sibling->sibling_next = child->sibling_next;
    }
  }

  child->parent = NULL;
  child->sibling_next = NULL;

  irq_enable();
}

struct task* find_child_by_pid(struct task* parent, int32_t pid) {
  if (!parent) return NULL;

  irq_disable();

  struct task* child = parent->first_child;
  while (child) {
    if (child->pid == pid) {
      irq_enable();
      return child;
    }
    child = child->sibling_next;
  }

  irq_enable();
  return NULL;
}

#include "kernel/task.h"

#include <stddef.h>

#include "kernel/printk.h"
#include "kernel/string.h"
#include "mm/kmap.h"
#include "mm/page.h"
#include "mm/user_pgtable.h"
#include "task_internal.h"

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

void destroy_task(struct task* task) {
  if (!task) {
    return;
  }

  printk("Destroying task PID=");
  printk_hex_u64(task->pid);
  printk("\n");

  if (task->mode == TASK_MODE_USER && task->ttbr0) {
    printk("Freeing user page tables\n");
    free_user_pgd((uint64_t*)task->ttbr0);
    task->ttbr0 = 0;
  }

  if (task->stack) {
    printk("Freeing kernel stack\n");
    // TODO: Make this a helper (in kmap.c)
    uint64_t stack_va = task->stack;
    uint64_t stack_phys = stack_va - KERNEL_BASE;
    struct page* stack_page = phys_to_page(stack_phys);
    if (stack_page) {
      free_page(stack_page);
    }
    task->stack = 0;
  }

  printk("Freeing task structure\n");
  uint64_t task_va = (uint64_t)task;
  uint64_t task_phys = task_va - KERNEL_BASE;
  struct page* task_page = phys_to_page(task_phys);
  if (task_page) {
    free_page(task_page);
  }
}

#include "kernel/user_thread.h"

#include <stddef.h>
#include <stdint.h>

#include "kernel/printk.h"
#include "kernel/scheduler.h"
#include "kernel/task.h"
#include "mm/kmap.h"
#include "mm/page.h"
#include "mm/pgtable.h"
#include "mm/user_pgtable.h"
#include "pid.h"
#include "task_internal.h"

#define USER_STACK_SIZE 4096
#define USER_STACK_TOP 0x0000000080000000ULL  // 2 GiB

// Create a simple user process with static mapping
struct task* create_user_process(void* code, size_t code_size) {
  // Allocate task struct
  struct task* t = alloc_task();
  if (!t) {
    return NULL;
  }

  // Allocate user page directory
  uint64_t* pgd = alloc_user_pgd();
  if (!pgd) {
    // TODO: free task
    return NULL;
  }

  t->mode = TASK_MODE_USER;
  t->ttbr0 = (uint64_t)pgd;

  // Map code at 0x1000 (user space)
  uint64_t code_va = 0x1000;
  size_t code_pages = (code_size + PAGE_SIZE - 1) / PAGE_SIZE;

  for (size_t i = 0; i < code_pages; i++) {
    struct page* p = alloc_page();
    if (!p) {
      // TODO: cleanup
      return NULL;
    }

    uint64_t phys = page_to_phys(p);

    // Copy code to this page
    void* dest = kmap(p);
    size_t to_copy = code_size < PAGE_SIZE ? code_size : PAGE_SIZE;
    for (size_t j = 0; j < to_copy; j++) {
      ((char*)dest)[j] = ((char*)code)[j];
    }
    code += to_copy;
    code_size -= to_copy;

    // Map with user permissions, read + execute
    uint64_t attrs = PTE_AF | PTE_SH_INNER | PTE_ATTRINDX(1) | PTE_USER;

    if (map_user_page(pgd, code_va, phys, attrs) != 0) {
      // TODO: cleanup
      return NULL;
    }

    code_va += PAGE_SIZE;
  }

  // Allocate user stack
  struct page* stack_page = alloc_page();
  if (!stack_page) {
    // TODO: cleanup
    return NULL;
  }

  uint64_t stack_phys = page_to_phys(stack_page);

  // Map stack with user permissions, read + write (no execute)
  uint64_t stack_attrs =
      PTE_AF | PTE_SH_INNER | PTE_ATTRINDX(1) | PTE_USER | PTE_UXN | PTE_PXN;

  uint64_t stack_va = USER_STACK_TOP - PAGE_SIZE;
  if (map_user_page(pgd, stack_va, stack_phys, stack_attrs) != 0) {
    // TODO: cleanup
    return NULL;
  }

  // Allocate kernel stack for this task
  void* kstack = alloc_stack();
  if (!kstack) {
    // TODO: cleanup
    return NULL;
  }

  // Set up context to enter user mode
  t->context.sp = (uint64_t)kstack + 4096;
  t->context.lr = 0x1000;  // User entry point

  t->state = TASK_READY;
  t->pid = pid_alloc();
  t->fn = NULL;
  t->arg = NULL;
  t->stack = (uint64_t)kstack;
  t->time_slice = DEFAULT_TIME_SLICE;

  enqueue_task(t);
  return t;
}

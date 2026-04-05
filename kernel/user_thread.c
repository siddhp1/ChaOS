#include "kernel/user_thread.h"

#include <stddef.h>
#include <stdint.h>

#include "kernel/kthread.h"
#include "kernel/pid.h"
#include "kernel/process.h"
#include "kernel/scheduler/scheduler.h"
#include "kernel/string.h"
#include "kernel/task.h"
#include "mm/kmap.h"
#include "mm/mmu.h"
#include "mm/page.h"
#include "mm/pgtable.h"
#include "mm/user_pgtable.h"

#define USER_STACK_SIZE 4096
#define USER_STACK_TOP USER_VIRT_END

extern void enter_usermode(uint64_t pc, uint64_t sp);

static void user_mode_entry(void* arg) {
  (void)arg;
  struct task* t = current_task;

  set_ttbr0(t->ttbr0);

  enter_usermode(USER_VIRT_ENTRY, USER_STACK_TOP);

  while (1);
}

struct task* create_user_process(void* code, size_t code_size) {
  struct task* t = alloc_task();
  if (!t) {
    return NULL;
  }

  t->parent = current_task;
  if (current_task) {
    add_child(current_task, t);
  }

  uint64_t* pgd = (uint64_t*)alloc_page_table();
  if (!pgd) {
    // TODO: Free task
    return NULL;
  }

  t->mode = TASK_MODE_USER;
  t->ttbr0 = (uint64_t)pgd;

  if (load_user_image(t, code, code_size) != 0) {
    // TODO: Free task
    return NULL;
  }

  struct page* stack_page = alloc_page();
  if (!stack_page) {
    // TODO: Cleanup
    return NULL;
  }

  uint64_t stack_phys = page_to_phys(stack_page);

  uint64_t stack_attrs =
      PTE_AF | PTE_SH_INNER | PTE_ATTRINDX(1) | PTE_USER | PTE_UXN | PTE_PXN;

  uint64_t stack_va = USER_STACK_TOP - PAGE_SIZE;
  if (map_user_page(pgd, stack_va, stack_phys, stack_attrs) != 0) {
    // TODO: Cleanup
    return NULL;
  }

  void* kstack = alloc_stack();
  if (!kstack) {
    // TODO: Cleanup
    return NULL;
  }

  t->irq_sp = (uint64_t)NULL;

  t->sp_el0 = USER_STACK_TOP;

  t->context.sp = (uint64_t)kstack + 4096;
  t->context.lr = (uint64_t)user_mode_entry;

  t->state = TASK_READY;
  t->pid = pid_alloc();
  t->fn = user_mode_entry;
  t->arg = NULL;
  t->stack = (uint64_t)kstack;
  t->time_slice = DEFAULT_TIME_SLICE;

  create_irq_frame(t);

  enqueue_task(t);
  return t;
}

int load_user_image(struct task* t, const void* code, size_t code_size) {
  if (!t || !code || code_size == 0) return -1;

  uint64_t* new_pgd = (uint64_t*)alloc_page_table();
  if (!new_pgd) return -1;

  const uint8_t* src = (const uint8_t*)code;
  size_t remaining = code_size;
  uint64_t va = USER_VIRT_ENTRY;

  while (remaining > 0) {
    struct page* p = alloc_page();
    if (!p) {
      free_user_pgd((uintptr_t)new_pgd);
      return -1;
    }

    void* dst = kmap(p);
    size_t chunk = (remaining > PAGE_SIZE) ? PAGE_SIZE : remaining;

    memset(dst, 0, PAGE_SIZE);
    memcpy(dst, src, chunk);

    uint64_t pa = page_to_phys(p);
    uint64_t attrs =
        PTE_PAGE | PTE_AF | PTE_SH_INNER | PTE_ATTRINDX(1) | PTE_USER;

    if (map_user_page(new_pgd, va, pa, attrs) != 0) {
      free_page(p);
      free_user_pgd((uintptr_t)new_pgd);
      return -1;
    }

    src += chunk;
    remaining -= chunk;
    va += PAGE_SIZE;
  }

  struct page* sp = alloc_page();
  if (!sp) {
    free_user_pgd((uintptr_t)new_pgd);
    return -1;
  }

  uint64_t spa = page_to_phys(sp);
  uint64_t stack_attrs = PTE_PAGE | PTE_AF | PTE_SH_INNER | PTE_ATTRINDX(1) |
                         PTE_USER | PTE_UXN | PTE_PXN;

  if (map_user_page(new_pgd, USER_STACK_TOP - PAGE_SIZE, spa, stack_attrs) !=
      0) {
    free_page(sp);
    free_user_pgd((uintptr_t)new_pgd);
    return -1;
  }

  uint64_t* old_pgd = (uint64_t*)t->ttbr0;
  t->ttbr0 = (uint64_t)new_pgd;
  t->sp_el0 = USER_STACK_TOP;

  if (t == current_task) {
    set_ttbr0((uintptr_t)new_pgd);
  }

  if (old_pgd) {
    free_user_pgd((uintptr_t)old_pgd);
  }

  return 0;
}

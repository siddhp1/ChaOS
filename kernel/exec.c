#include "kernel/exec.h"

#include <stddef.h>

#include "kernel/initramfs.h"
#include "kernel/printk.h"
#include "kernel/scheduler.h"
#include "kernel/string.h"
#include "kernel/task.h"
#include "kernel/trap.h"
#include "kernel/vm.h"
#include "mm/kmap.h"
#include "mm/page.h"
#include "task_internal.h"

long sys_execve(long filepath, long a1, long a2, long a3, long a4, long a5) {
  (void)a1;
  (void)a2;
  (void)a3;
  (void)a4;
  (void)a5;

  const char *path = (const char *)filepath;
  struct task *t = current_task;

  struct initramfs_file *f = initramfs_lookup(path);
  if (!f) {
    printk("exec: not found: ");
    printk(path);
    printk("\n");
    return -1;
  }

  if (t->ttbr0) {
    vm_destroy_user(t);
  }

  t->ttbr0 = vm_create_user_ttbr0();
  if (!t->ttbr0) {
    printk("exec: failed to create user page table\n");
    return -1;
  }

  uint64_t va = USER_BASE;
  size_t remaining = f->size;
  char *src = (char *)f->data;
  t->user_page_count = 0;

  while (remaining > 0) {
    struct page *p = alloc_page();
    if (!p) return -1;

    void *kptr = kmap(p);
    memset(kptr, 0, PAGE_SIZE);

    size_t chunk = remaining < PAGE_SIZE ? remaining : PAGE_SIZE;
    memcpy(kptr, src, chunk);

    vm_map_user_page(t->ttbr0, va, p, VM_USER_RWX);
    t->user_pages[t->user_page_count++] = p;

    va += PAGE_SIZE;
    src += chunk;
    remaining -= chunk;
  }

  {
    struct page *sp_page = alloc_page();
    if (!sp_page) return -1;
    memset(kmap(sp_page), 0, PAGE_SIZE);
    vm_map_user_page(t->ttbr0, USER_STACK_TOP - PAGE_SIZE, sp_page, VM_USER_RW);
    t->user_pages[t->user_page_count++] = sp_page;
  }

  t->user_entry = USER_BASE;
  t->user_sp = USER_STACK_TOP;
  t->mode = TASK_MODE_USER;

  asm volatile(
      "msr ttbr0_el1, %0  \n"
      "isb                \n"
      "tlbi vmalle1is     \n"
      "dsb ish            \n"
      "isb                \n"
      :
      : "r"(t->ttbr0)
      : "memory");

  asm volatile("msr sp_el0, %0" : : "r"(USER_STACK_TOP));

  // Set up initial trapframe for the new user program
  struct trapframe *tf = (struct trapframe *)t->irq_sp;
  if (tf) {
    for (int i = 0; i < 31; i++) tf->x[i] = 0;
    tf->elr_el1 = USER_BASE;
    tf->spsr_el1 = 0;
  }

  return 0;
}

void load_init(void) {
  struct task *init_task = alloc_task();
  if (!init_task) {
    printk("Failed to allocate init task\n");
    return;
  }

  init_task->mode = TASK_MODE_KERNEL;

  current_task = init_task;
  sys_execve((long)"bin/init", 0, 0, 0, 0, 0);
}

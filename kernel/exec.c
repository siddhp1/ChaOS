#include "kernel/exec.h"

#include <stddef.h>

#include "kernel/initramfs.h"
#include "kernel/kthread.h"
#include "kernel/printk.h"
#include "kernel/scheduler.h"
#include "kernel/string.h"
#include "kernel/task.h"
#include "kernel/trap.h"
#include "kernel/uaccess.h"
#include "kernel/uthread.h"
#include "kernel/vm.h"
#include "mm/kmap.h"
#include "mm/page.h"
#include "task_internal.h"

#define EXEC_PATH_MAX 128

static long copy_user_cstr(char* dst, size_t dst_len, const char* user_src) {
  if (!dst || !user_src || dst_len == 0) return -1;

  for (size_t i = 0; i < dst_len; i++) {
    char c = 0;
    if (copy_from_user(&c, user_src + i, 1) < 0) {
      return -1;
    }
    dst[i] = c;
    if (c == '\0') return 0;
  }

  // Not null-terminated within limit
  dst[dst_len - 1] = '\0';
  return -1;
}

long execve(const char* kpath) {
  if (!kpath || kpath[0] == '\0') {
    return -1;
  }

  // const char* path = (const char*)filepath;
  struct task* t = current_task;

  struct initramfs_file* f = initramfs_lookup(kpath);
  if (!f) {
    printk("exec: not found: ");
    printk(kpath);
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
  char* src = (char*)f->data;
  t->user_page_count = 0;

  while (remaining > 0) {
    struct page* p = alloc_page();
    if (!p) return -1;

    void* kptr = kmap(p);
    memset(kptr, 0, PAGE_SIZE);

    size_t chunk = remaining < PAGE_SIZE ? remaining : PAGE_SIZE;
    memcpy(kptr, src, chunk);

    vm_map_user_page(t->ttbr0, va, p, VM_USER_RWX);

    t->user_pages[t->user_page_count] = p;
    t->user_page_vas[t->user_page_count] = va;
    t->user_page_count++;

    va += PAGE_SIZE;
    src += chunk;
    remaining -= chunk;
  }

  {
    struct page* sp_page = alloc_page();
    if (!sp_page) return -1;
    memset(kmap(sp_page), 0, PAGE_SIZE);
    vm_map_user_page(t->ttbr0, USER_STACK_TOP - PAGE_SIZE, sp_page, VM_USER_RW);
    t->user_pages[t->user_page_count] = sp_page;
    t->user_page_vas[t->user_page_count] = USER_STACK_TOP - PAGE_SIZE;
    t->user_page_count++;
  }

  t->user_entry = USER_BASE;
  t->user_sp = USER_STACK_TOP;
  t->mode = TASK_MODE_USER;

  // asm volatile(
  //     "msr ttbr0_el1, %0  \n"
  //     "isb                \n"
  //     "tlbi vmalle1is     \n"
  //     "dsb ish            \n"
  //     "isb                \n"
  //     :
  //     : "r"(t->ttbr0)
  //     : "memory");

  // asm volatile("msr ttbr0_el1, %0" : : "r"(t->ttbr0));
  // asm volatile("isb");
  // asm volatile("tlbi vmalle1is");
  // asm volatile("dsb ish");
  // asm volatile("isb");

  // asm volatile("msr sp_el0, %0" : : "r"(USER_STACK_TOP));

  // // Set up initial trapframe for the new user program
  // struct trapframe* tf = (struct trapframe*)t->irq_sp;
  // if (tf) {
  //   for (int i = 0; i < 31; i++) tf->x[i] = 0;
  //   tf->elr_el1 = USER_BASE;
  //   tf->spsr_el1 = 0;
  // }
  t->irq_sp = 0;

  return 0;
}

long kernel_execve(const char* filepath) { return execve(filepath); }

long sys_execve(long filepath, long a1, long a2, long a3, long a4, long a5) {
  (void)a1;
  (void)a2;
  (void)a3;
  (void)a4;
  (void)a5;

  const char* user_path = (const char*)filepath;
  char kpath[EXEC_PATH_MAX];

  if (copy_user_cstr(kpath, EXEC_PATH_MAX, user_path) < 0) {
    return -1;
  }

  return execve(kpath);
}

void load_init(void) {
  struct task* init_task = kthread_create(NULL, NULL);
  if (!init_task) {
    printk("Failed to allocate init task\n");
    return;
  }

  struct task* idle = current_task;
  current_task = init_task;

  // int32_t pid = uthread_create(0, 0, 0);
  // if (pid < 0) {
  //   printk("Failed to allocate init task\n");
  //   return;
  // }

  long rc = kernel_execve("bin/init");
  current_task = idle;

  if (rc < 0) {
    printk("Failed to load init");
    return;
  }

  // init_task->state = TASK_READY;
  // enqueue_task(init_task);
  need_schedule = true;
  // sys_execve((long)"bin/init", 0, 0, 0, 0, 0);
}

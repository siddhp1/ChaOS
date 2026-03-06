#include "kernel/fork.h"

#include <stddef.h>

#include "kernel/printk.h"
#include "kernel/scheduler.h"
#include "kernel/string.h"
#include "kernel/task.h"
#include "kernel/trap.h"
#include "kernel/vm.h"
#include "mm/kmap.h"
#include "mm/page.h"
#include "pid.h"
#include "task_internal.h"

#define KSTACK_SIZE 4096

long sys_fork(long a0, long a1, long a2, long a3, long a4, long a5) {
  (void)a0;
  (void)a1;
  (void)a2;
  (void)a3;
  (void)a4;
  (void)a5;

  struct task* parent = current_task;
  if (!parent) return -1;

  if (parent->irq_sp == 0) return -1;

  struct task* child = alloc_task();
  if (!child) return -1;

  void* stack_base = alloc_stack();
  if (!stack_base) return -1;

  memset(&child->context, 0, sizeof(child->context));
  child->context.sp = (uint64_t)stack_base + KSTACK_SIZE;
  child->context.lr = parent->context.lr;

  child->stack = (uint64_t)stack_base;
  child->irq_sp = 0;
  child->state = TASK_READY;
  child->pid = pid_alloc();
  child->mode = TASK_MODE_USER;
  child->time_slice = DEFAULT_TIME_SLICE;
  child->fn = NULL;
  child->arg = NULL;
  child->wakeup_tick = 0;
  child->parent = parent;
  child->next = NULL;

  child->ttbr0 = vm_create_user_ttbr0();
  if (!child->ttbr0) return -1;

  child->user_page_count = 0;
  for (uint64_t i = 0; i < parent->user_page_count; i++) {
    struct page* src_page = parent->user_pages[i];
    struct page* dst_page = alloc_page();
    if (!dst_page) {
      return -1;
    }

    void* dst_kva = kmap(dst_page);
    void* src_kva = kmap(src_page);
    memcpy(dst_kva, src_kva, PAGE_SIZE);

    uint64_t va = parent->user_page_vas[i];
    vm_map_user_page(child->ttbr0, va, dst_page, VM_USER_RWX);
    child->user_pages[child->user_page_count] = dst_page;
    child->user_page_vas[child->user_page_count] = va;
    child->user_page_count++;
  }

  child->user_entry = parent->user_entry;
  child->user_sp = parent->user_sp;

  uint64_t child_frame_sp = child->context.sp - sizeof(struct trapframe);
  struct trapframe* child_tf = (struct trapframe*)child_frame_sp;
  struct trapframe* parent_tf = (struct trapframe*)parent->irq_sp;

  memcpy(child_tf, parent_tf, sizeof(struct trapframe));
  child_tf->x[0] = 0;  // returns 0 in child

  child->irq_sp = child_frame_sp;
  child->context.sp = child_frame_sp;

  enqueue_task(child);
  return child->pid;  // returns child PID in parent
}

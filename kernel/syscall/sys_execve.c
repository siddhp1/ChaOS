#include <stdint.h>

#include "kernel/initramfs.h"
#include "kernel/irq_frame.h"
#include "kernel/scheduler/scheduler.h"
#include "kernel/task.h"
#include "kernel/user_access.h"
#include "kernel/user_thread.h"
#include "mm/mmu.h"
#include "mm/page.h"
#include "mm/pgtable.h"
#include "mm/user_pgtable.h"
#include "syscall_handlers.h"

#define EXEC_PATH_MAX 128
#define USER_STACK_TOP 0x0000000080000000ULL  // 2 GiB

static long copy_user_cstr(char* dst, const char* user_src, size_t max_len) {
  if (!dst || !user_src || max_len == 0) return -1;

  for (size_t i = 0; i < max_len; i++) {
    char c = 0;
    if (copy_from_user(&c, user_src + i, 1) < 0) return -1;
    dst[i] = c;
    if (c == '\0') return (long)i;
  }

  dst[max_len - 1] = '\0';
  return -1;  // No terminator in bound
}

long sys_execve(long pathname, long argv, long envp, long a3, long a4,
                long a5) {
  (void)argv;
  (void)envp;
  (void)a3;
  (void)a4;
  (void)a5;

  if (pathname == 0) return -1;
  if (!current_task) return -1;
  if (current_task->mode != TASK_MODE_USER) return -1;

  char path[EXEC_PATH_MAX];
  if (copy_user_cstr(path, (const char*)pathname, sizeof(path)) < 0) return -1;

  struct initramfs_file* f = initramfs_lookup(path);

  // Allow leading slash
  if (!f && path[0] == '/') {
    f = initramfs_lookup(path + 1);
  }

  if (!f || !f->data || f->size == 0) return -1;

  if (load_user_image(current_task, f->data, f->size) != 0) return -1;

  trapframe* frame = (trapframe*)current_task->irq_sp;
  if (!frame) return -1;

  frame->elr_el1 = USER_VIRT_ENTRY;
  frame->sp_el0 = USER_STACK_TOP;
  frame->x[0] = 0;

  set_ttbr0(current_task->ttbr0);

  return 0;
}

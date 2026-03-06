#ifndef TASK_H
#define TASK_H

#include <stdbool.h>
#include <stdint.h>

#include "kernel/cpu_context.h"

#define MAX_USER_PAGES 240  // must keep sizeof(struct task) <= PAGE_SIZE (4096)

enum task_mode { TASK_MODE_KERNEL, TASK_MODE_USER };

enum task_state {
  TASK_READY,
  TASK_RUNNING,
  TASK_BLOCKED,
  TASK_ZOMBIE,
  TASK_SLEEPING
};

// TODO: Rename struct members for better kernel/user separation
struct task {
  struct cpu_context context;

  enum task_state state;
  int32_t pid;

  void (*fn)(void*);
  void* arg;

  uint64_t stack;

  // Saved irq frame base (sp after sub sp, sp, #irq_frame_size)
  uint64_t irq_sp;

  int32_t time_slice;
  uint64_t wakeup_tick;

  enum task_mode mode;
  uint64_t user_entry;  // EL0 entry VA
  uint64_t user_sp;     // EL0 stack top VA
  uint64_t ttbr0;       // Per-task user page table root PA
  uint64_t user_page_count;
  struct task* parent;
  struct page* user_pages[MAX_USER_PAGES];
  uint64_t user_page_vas[MAX_USER_PAGES];

  struct task* next;
};

_Static_assert(sizeof(struct task) <= 4096,
               "struct task must fit in a single 4K page (see alloc_task)");

// TODO: Determine if needed
static inline bool task_is_user(const struct task* t) {
  return t && t->mode == TASK_MODE_USER;
}

void set_task_state(struct task* task, enum task_state new_state);

// TODO: Correct implementation
// void destroy_task(struct task* task);

#endif

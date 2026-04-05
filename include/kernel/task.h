#ifndef TASK_H
#define TASK_H

#include <stdint.h>

#include "kernel/cpu_context.h"

enum task_mode { TASK_MODE_KERNEL, TASK_MODE_USER };

enum task_state {
  TASK_READY,
  TASK_RUNNING,
  TASK_BLOCKED,
  TASK_ZOMBIE,
  TASK_SLEEPING
};

struct task {
  struct cpu_context context;

  enum task_state state;
  int32_t pid;

  void (*fn)(void*);
  void* arg;

  uint64_t stack;

  // saved irq frame base (sp after sub sp, sp, #irq_frame_size)
  uint64_t irq_sp;

  int32_t time_slice;
  uint64_t wakeup_tick;

  enum task_mode mode;
  uint64_t ttbr0;
  uint64_t sp_el0;

  int32_t exit_status;

  struct task* parent;
  struct task* first_child;
  struct task* sibling_next;

  struct task* next;
};

void* alloc_stack(void);
struct task* alloc_task(void);
void create_irq_frame(struct task* task);

void set_task_state(struct task* task, enum task_state new_state);
void destroy_task(struct task* task);

#endif

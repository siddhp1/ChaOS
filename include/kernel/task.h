#ifndef TASK_H
#define TASK_H

#include <stdint.h>

// Number of hardware timer interrupts before scheduling
#define DEFAULT_TIME_SLICE 1

#define KSTACK_SIZE 4096

enum task_mode { TASK_MODE_KERNEL, TASK_MODE_USER };

enum task_state {
  TASK_READY,
  TASK_RUNNING,
  TASK_BLOCKED,
  TASK_ZOMBIE,
  TASK_SLEEPING
};

struct task {
  enum task_state state;
  int32_t pid;

  void (*fn)(void*);
  void* arg;

  uintptr_t stack;
  uintptr_t irq_sp;

  int32_t time_slice;
  uint64_t wakeup_tick;

  enum task_mode mode;
  uintptr_t ttbr0;
  uintptr_t sp_el0;

  int32_t exit_status;

  struct task* parent;
  struct task* first_child;
  struct task* sibling_next;

  struct task* next;
};

void* alloc_stack(void);
struct task* alloc_task(void);
void create_irq_frame(struct task* task, uintptr_t stack_top,
                      uintptr_t entry_fn);
void destroy_task(struct task* task);

#endif

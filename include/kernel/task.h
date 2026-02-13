#ifndef TASK_H
#define TASK_H

#include <stdint.h>

#include "kernel/cpu_context.h"

enum task_state { TASK_READY, TASK_RUNNING, TASK_BLOCKED, TASK_ZOMBIE };

struct task {
  struct cpu_context context;

  enum task_state state;
  int32_t pid;

  void (*fn)(void*);
  void* arg;

  uint64_t stack;

  struct task* next;
};

void set_task_state(struct task* task, enum task_state new_state);

// TODO: Correct implementation
// void destroy_task(struct task* task);

#endif

#ifndef TASK_H
#define TASK_H

#include <stdint.h>

#include "kernel/context.h"

typedef enum {
  TASK_READY,
  TASK_RUNNING,
  TASK_BLOCKED,
  TASK_ZOMBIE
} task_state_t;

typedef struct task {
  task_state_t state;
  uint32_t pid;
  context_t context;
  uintptr_t stack_page;
  struct task* next;
} task_t;

task_t* create_task(uintptr_t entry_point);
void set_task_state(task_t* task, task_state_t new_state);
void destroy_task(task_t* task);

#endif

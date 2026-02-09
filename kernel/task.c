#include "kernel/task.h"

#include <stddef.h>

#include "kernel/context.h"
#include "kernel/printk.h"
#include "mm/page.h"

static uint64_t next_pid = 1;

task_t* create_task(uintptr_t entry_point) {
  task_t* task = (task_t*)alloc_page();
  if (!task) {
    printk("Failed to allocate task\n");
    return NULL;
  }

  struct page* stack_page = alloc_page();
  if (!stack_page) {
    printk("Failed to allocate stack page for task\n");
    free_page((struct page*)task);
    return NULL;
  }
  uintptr_t stack_top = page_to_phys(stack_page) + PAGE_SIZE;

  task->stack_page = (uintptr_t)stack_page;
  task->state = TASK_READY;
  task->pid = next_pid++;
  context_init(&task->context, entry_point, stack_top);
  task->next = NULL;

  return task;
}

void set_task_state(task_t* task, task_state_t new_state) {
  if (task) {
    task->state = new_state;
  }
}

void destroy_task(task_t* task) {
  if (!task) return;
  free_page((struct page*)task->stack_page);
  free_page((struct page*)task);
}

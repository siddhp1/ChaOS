#include "kernel/task.h"

#include <stddef.h>

#include "kernel/printk.h"
#include "kernel/string.h"
#include "mm/kmap.h"
#include "mm/page.h"
#include "task_internal.h"

void* alloc_stack(void) {
  struct page* p = alloc_page();
  if (!p) {
    return NULL;
  }

  void* va = kmap(p);
  return va;  // Bottom of stack page
}

struct task* alloc_task(void) {
  struct page* p = alloc_page();
  if (!p) {
    return NULL;
  }

  struct task* t = (struct task*)kmap(p);

  memset(t, 0, sizeof(*t));

  return t;
}

// TODO: Correct implementation
// void destroy_task(struct task* task) {
//   if (!task) {
//     return;
//   }

//   free_page((struct page*)task->stack_page);
//   free_page((struct page*)task->task_page);
// }

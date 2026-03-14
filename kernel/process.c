#include "kernel/process.h"

#include <stddef.h>

#include "kernel/irq.h"
#include "kernel/task.h"

void add_child(struct task* parent, struct task* child) {
  if (!parent || !child) return;

  irq_disable();

  child->parent = parent;
  child->sibling_next = parent->first_child;
  parent->first_child = child;

  irq_enable();
}

void remove_child(struct task* parent, struct task* child) {
  if (!parent || !child) return;

  irq_disable();

  // TODO: Make helper
  if (parent->first_child == child) {
    parent->first_child = child->sibling_next;
  } else {
    struct task* sibling = parent->first_child;
    while (sibling && sibling->sibling_next != child) {
      sibling = sibling->sibling_next;
    }
    if (sibling) {
      sibling->sibling_next = child->sibling_next;
    }
  }

  child->parent = NULL;
  child->sibling_next = NULL;

  irq_enable();
}

struct task* find_child_by_pid(struct task* parent, int32_t pid) {
  if (!parent) return NULL;

  irq_disable();

  struct task* child = parent->first_child;
  while (child) {
    if (child->pid == pid) {
      irq_enable();
      return child;
    }
    child = child->sibling_next;
  }

  irq_enable();
  return NULL;
}

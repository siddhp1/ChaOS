#include "kernel/reaper.h"

#include <stddef.h>

#include "kernel/irq.h"
#include "kernel/printk.h"
#include "kernel/scheduler.h"
#include "kernel/task.h"

static struct task* zombie_queue = NULL;

void enqueue_zombie(struct task* task) {
  if (!task) return;

  irq_disable();

  task->next = zombie_queue;
  zombie_queue = task;

  irq_enable();
}

void task_zombie(struct task* task) {
  if (!task) {
    return;
  }

  current_task->state = TASK_ZOMBIE;
  enqueue_zombie(current_task);
}

void reap_zombies(void) {
  irq_disable();

  struct task* zombie = zombie_queue;
  zombie_queue = NULL;

  irq_enable();

  while (zombie) {
    struct task* next = zombie->next;

    printk("Reaping zombie PID=");
    printk_hex_u64(zombie->pid);
    printk("\n");

    destroy_task(zombie);

    zombie = next;
  }
}

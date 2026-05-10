#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "kernel/irq.h"
#include "kernel/printk.h"
#include "kernel/scheduler/scheduler.h"
#include "kernel/scheduler/wait.h"
#include "kernel/user_access.h"
#include "syscall_handlers.h"

long sys_waitpid(long pid, long status, long a2, long a3, long a4, long a5) {
  (void)a2;
  (void)a3;
  (void)a4;
  (void)a5;

  struct task* parent = current_task;

  while (1) {
    irq_disable();

    struct task* prev = NULL;
    struct task* child = parent->first_child;

    while (child) {
      if ((pid == -1 || child->pid == (int32_t)pid) &&
          child->state == TASK_ZOMBIE) {
        if (prev) {
          prev->sibling_next = child->sibling_next;
        } else {
          parent->first_child = child->sibling_next;
        }
        child->sibling_next = NULL;
        child->parent = NULL;

        int32_t child_pid = child->pid;
        int32_t child_status = child->exit_status;

        irq_enable();

        printk("sys_waitpid: Reaping child PID=%u of parent PID=%u\n",
               child_pid, parent->pid);
        destroy_task(child);

        if (status) {
          if (copy_to_user((void*)status, &child_status, sizeof(int32_t)) < 0) {
            return -1;
          }
        }
        return child_pid;
      }

      prev = child;
      child = child->sibling_next;
    }

    if (pid > 0) {
      struct task* c = parent->first_child;
      bool exists = false;
      while (c) {
        if (c->pid == (int32_t)pid) {
          exists = true;
          break;
        }
        c = c->sibling_next;
      }
      if (!exists) {
        irq_enable();
        return -1;
      }
    }

    task_wait(&parent->wait_child_queue, parent);
    irq_enable();
  }
}

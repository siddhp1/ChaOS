#include <stdint.h>

#include "kernel/cpu.h"
#include "kernel/initramfs.h"
#include "kernel/irq.h"
#include "kernel/kthread.h"
#include "kernel/printk.h"
#include "kernel/scheduler.h"
#include "kernel/sleep.h"
#include "kernel/task.h"
#include "kernel/uart.h"
#include "kernel/user_thread.h"
#include "kernel/wait.h"
#include "mm/heap.h"
#include "mm/kmap.h"
#include "mm/memory.h"
#include "mm/mmu.h"
#include "mm/page.h"
#include "mm/pgtable.h"

#define DELAY_CYCLES 10000000
#define SLEEP_TICKS 100

extern void context_switch(struct cpu_context* a, struct cpu_context* b);

static struct cpu_context boot_context;

void thread_a(void* arg) {
  while (1) {
    printk("A\n");
    for (volatile int i = 0; i < DELAY_CYCLES; i++);
  }
}

void thread_b(void* arg) {
  while (1) {
    printk("B\n");
    for (volatile int i = 0; i < DELAY_CYCLES; i++);
  }
}

void thread_sleep_test(void* arg) {
  while (1) {
    printk("Going to sleep\n");
    task_sleep(SLEEP_TICKS, current_task);
    printk("Woke up!\n");
  }
}

void thread_wait_test(void* arg) {
  while (1) {
    printk("Waiting for event\n");
    wait_event(current_task);
    printk("Got event!\n");
  }
}

void thread_waker(void* arg) {
  while (1) {
    for (volatile int i = 0; i < DELAY_CYCLES * 5; i++);
    printk("Waking up wait queue\n");
    wake_up();
  }
}

void kernel_entry(void) {
  uart_init();
  printk("UART initialized\n");

  exception_init();
  printk("Exceptions initialized\n");

  irq_init();
  printk("IRQ initialized\n");

  memory_init();
  printk("Memory initialized\n");

  printk("Testing L3 page allocation...\n");

  void* p1 = kmalloc(1);
  void* p2 = kmalloc(1);
  void* p3 = kmalloc(1);

  printk("Allocated p1=%lx\n", (uint64_t)p1);
  printk("Allocated p2=%lx\n", (uint64_t)p2);
  printk("Allocated p3=%lx\n", (uint64_t)p3);

  *(char*)p1 = 'A';
  *(char*)p2 = 'B';
  *(char*)p3 = 'C';

  printk("Write test passed\n");

  scheduler_init();
  printk("Scheduler initialized\n");

  struct task* t1 = kthread_create(thread_a, NULL);
  struct task* t2 = kthread_create(thread_b, NULL);
  struct task* t3 = kthread_create(thread_sleep_test, NULL);
  struct task* t4 = kthread_create(thread_wait_test, NULL);
  struct task* t5 = kthread_create(thread_waker, NULL);

  initramfs_init();
  printk("Initramfs initialized\n");

  struct task* init_task = load_init();
  if (!init_task) {
    printk("Cannot launch init\n");
    while (1) asm volatile("wfi");
  }

  // TODO: Organize
  switch_user_pgd((uint64_t*)current_task->ttbr0);
  context_switch(&boot_context, &current_task->context);

  while (1) asm volatile("wfi");  // Unreachable
}

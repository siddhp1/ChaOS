#include <stdint.h>

#include "kernel/cpu.h"
#include "kernel/exec.h"
#include "kernel/initramfs.h"
#include "kernel/irq.h"
#include "kernel/kthread.h"
#include "kernel/printk.h"
#include "kernel/scheduler.h"
#include "kernel/sleep.h"
#include "kernel/string.h"
#include "kernel/task.h"
#include "kernel/uart.h"
#include "kernel/wait.h"
#include "mm/heap.h"
#include "mm/kmap.h"
#include "mm/memory.h"
#include "mm/mmu.h"
#include "mm/page.h"

#define DELAY_CYCLES 10000000
#define SLEEP_TICKS 100

extern uintptr_t bss_start;
extern uintptr_t bss_end;

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
  // Zero out the BSS (static variables)
  volatile uintptr_t* ptr = (volatile uintptr_t*)&bss_start;
  while (ptr < &bss_end) {
    *ptr++ = 0;
  }

  uart_init();
  printk("UART initialized\n");

  exception_init();
  printk("Exceptions initialized\n");

  irq_init();
  printk("IRQ initialized\n");

  memory_init();
  printk("Memory initialized\n");

  initramfs_init();
  printk("Initramfs initialized\n");

  scheduler_init();
  printk("Scheduler initialized\n");

  // struct task* t1 = kthread_create(thread_a, NULL);
  // struct task* t2 = kthread_create(thread_b, NULL);

  // Load PID 1 (bin/init) from initramfs
  load_init();

  context_switch(&boot_context, &current_task->context);

  while (1) asm volatile("WFI");  // Unreachable
}

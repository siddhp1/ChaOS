#include <stdint.h>

#include "kernel/cpu.h"
#include "kernel/irq.h"
#include "kernel/kthread.h"
#include "kernel/printk.h"
#include "kernel/scheduler.h"
#include "kernel/task.h"
#include "kernel/uart.h"
#include "mm/heap.h"
#include "mm/kmap.h"
#include "mm/memory.h"
#include "mm/mmu.h"
#include "mm/page.h"

#define DELAY_CYCLES 10000000

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

  scheduler_init();
  printk("Scheduler initialized\n");

  struct task* t1 = kthread_create(thread_a, NULL);
  struct task* t2 = kthread_create(thread_b, NULL);

  // TODO: Move out of main
  context_switch(&boot_context, &current_task->context);

  while (1) asm volatile("WFI");  // Unreachable
}

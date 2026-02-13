#include "timer.h"

#include <stdint.h>

#include "irq_internal.h"
#include "kernel/printk.h"

#define TIMER_IRQ 27
#define TIMER_INTERVAL 10000000

void timer_init(void) {
  // Program timer to fire TIMER_INTERVAL ticks from now
  asm volatile("msr CNTV_TVAL_EL0, %0"
               :
               : "r"((uint64_t)TIMER_INTERVAL)
               : "memory");

  // Enable timer
  asm volatile("msr CNTV_CTL_EL0, %0" : : "r"((uint64_t)1) : "memory");

  register_irq(TIMER_IRQ, timer_interrupt);
}

void timer_interrupt(void* unused) {
  // Re-arm timer for the next tick
  asm volatile("msr CNTV_TVAL_EL0, %0"
               :
               : "r"((uint64_t)TIMER_INTERVAL)
               : "memory");

  printk("TICK\n");
}

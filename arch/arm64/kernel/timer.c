#include "timer.h"

#include <stdint.h>

#include "irq_internal.h"
#include "kernel/printk.h"
#include "kernel/scheduler/scheduler.h"
#include "kernel/scheduler/sleep.h"

#define TIMER_IRQ 27
// TODO: Increase frequency
#define TIMER_HZ 10

static uint64_t timer_interval_ticks;

void timer_init(void) {
  // Read counter frequency register
  uint64_t cntfrq;
  asm volatile("mrs %0, CNTFRQ_EL0" : "=r"(cntfrq));

  // Round up to prevent 0 tick interval
  timer_interval_ticks = (cntfrq + TIMER_HZ - 1) / TIMER_HZ;
  if (timer_interval_ticks == 0) {
    timer_interval_ticks = 1;
  }

  // Program timer to fire timer_interval_ticks from now
  asm volatile("msr CNTV_TVAL_EL0, %0"
               :
               : "r"(timer_interval_ticks)
               : "memory");

  // Enable timer
  asm volatile("msr CNTV_CTL_EL0, %0" : : "r"((uint64_t)1) : "memory");

  register_irq(TIMER_IRQ, timer_interrupt);
}

void timer_interrupt(void* unused) {
  // Re-arm timer for the next tick
  asm volatile("msr CNTV_TVAL_EL0, %0"
               :
               : "r"((uint64_t)timer_interval_ticks)
               : "memory");

  printk("TICK\n");
  scheduler_tick();
  system_tick++;
  check_sleeping_tasks();
}

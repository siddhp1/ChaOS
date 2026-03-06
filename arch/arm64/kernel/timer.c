#include "timer.h"

#include <stdint.h>

#include "irq_internal.h"
#include "kernel/printk.h"
#include "kernel/scheduler.h"
#include "kernel/sleep.h"

#define TIMER_IRQ 27
// 100 Hz = 10ms tick interval for responsive preemption
#define TIMER_HZ 10

static uint64_t timer_interval_ticks;

void timer_init(void) {
  // Enable virtual timer access from EL0 and ensure no trapping
  // CNTKCTL_EL1: EL0VCTEN (bit 1) = 1, EL0VTEN (bit 0) = 1
  uint64_t cntkctl = 0x3;  // Enable EL0 access to virtual timer
  asm volatile("msr CNTKCTL_EL1, %0" : : "r"(cntkctl) : "memory");
  asm volatile("isb" ::: "memory");

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

  // Enable timer: ENABLE=1, IMASK=0
  asm volatile("msr CNTV_CTL_EL0, %0" : : "r"((uint64_t)1) : "memory");
  asm volatile("isb" ::: "memory");

  register_irq(TIMER_IRQ, timer_interrupt);
}

void timer_interrupt(void* unused) {
  (void)unused;

  // Re-arm timer for the next tick
  asm volatile("msr CNTV_TVAL_EL0, %0"
               :
               : "r"((uint64_t)timer_interval_ticks)
               : "memory");

  // printk("TICK\n");
  scheduler_tick();
  system_tick++;
  check_sleeping_tasks();
}

#include "timer.h"

#include <stdint.h>

#include "kernel/printk.h"

#define TIMER_INTERVAL 100000

void timer_init(void) {
  asm volatile("msr CNTV_TVAL_EL0, %0"
               :
               : "r"((uint64_t)TIMER_INTERVAL)
               : "memory");
  asm volatile("msr CNTV_CTL_EL0, %0" : : "r"((uint64_t)1) : "memory");
}

void timer_interrupt(void) {
  asm volatile("msr CNTV_TVAL_EL0, %0"
               :
               : "r"((uint64_t)TIMER_INTERVAL)
               : "memory");
  printk("tick");
  // schedule();
}

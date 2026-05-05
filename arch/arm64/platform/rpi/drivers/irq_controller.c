#include "irq_controller.h"

#include <stdint.h>

#include "kernel/irq.h"
#include "mm/kmap.h"

#define LOCAL_PERIPH_BASE (KERNEL_VIRT_BASE + 0x40000000UL)

#define CORE0_TIMER_IRQ_CTRL (*(volatile uint32_t*)(LOCAL_PERIPH_BASE + 0x40))
#define CORE0_IRQ_SOURCE (*(volatile uint32_t*)(LOCAL_PERIPH_BASE + 0x60))

#define CORE_IRQ_CNTPNS (1U << 1)  // Physical non-secure timer

#define RPI_TIMER_IRQ_ID 27U  // Same as GIC

void irq_controller_init(void) {
  CORE0_TIMER_IRQ_CTRL = CORE_IRQ_CNTPNS;
  asm volatile("dsb sy" ::: "memory");
  asm volatile("isb" ::: "memory");
}

uint32_t irq_controller_ack(void) {
  uint32_t src = CORE0_IRQ_SOURCE;

  if (src & CORE_IRQ_CNTPNS) {
    return RPI_TIMER_IRQ_ID;
  }

  return IRQ_NONE;
}

void irq_controller_eoi(uint32_t irq) {
  (void)irq;
  // No explicit EOI needed for local peripheral block
}

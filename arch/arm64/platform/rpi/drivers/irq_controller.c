#include "irq_controller.h"

#include <stdint.h>

#include "kernel/irq.h"
#include "mm/kmap.h"

#define LOCAL_PERIPH_BASE (KERNEL_VIRT_BASE + 0x40000000UL)

#define CORE0_TIMER_IRQ_CTRL (*(volatile uint32_t*)(LOCAL_PERIPH_BASE + 0x40))
#define CORE_CNTPNS_IRQ_EN (1U << 1)  // Physical non-secure timer
#define CORE_IRQ_CNTPNS (1U << 1)     // CNTPNS interrupt pending

#define CORE0_MBOX_IRQ_CTRL (*(volatile uint32_t*)(LOCAL_PERIPH_BASE + 0x50))
#define CORE0_MBOX0_SET (*(volatile uint32_t*)(LOCAL_PERIPH_BASE + 0x80))
#define CORE0_MBOX0_CLR (*(volatile uint32_t*)(LOCAL_PERIPH_BASE + 0xC0))
#define CORE_MBOX0_IRQ_EN 1U      // Mailbox 0 IRQ
#define CORE_IRQ_MBOX0 (1U << 4)  // Mailbox 0 pending

#define CORE0_IRQ_SOURCE (*(volatile uint32_t*)(LOCAL_PERIPH_BASE + 0x60))

void irq_controller_init(void) {
  CORE0_TIMER_IRQ_CTRL = CORE_CNTPNS_IRQ_EN;
  CORE0_MBOX_IRQ_CTRL = CORE_MBOX0_IRQ_EN;
  asm volatile("dsb sy" ::: "memory");
  asm volatile("isb" ::: "memory");
}

void irq_controller_send_sgi(uint64_t sgi_id) {
  asm volatile("dsb sy" ::: "memory");
  CORE0_MBOX0_SET = (1U << 0);
}

uint32_t irq_controller_ack(void) {
  uint32_t src = CORE0_IRQ_SOURCE;

  if (src & CORE_IRQ_CNTPNS) {
    return IRQ_TIMER;
  }

  if (src & CORE_IRQ_MBOX0) {  // Mailbox 0 has bits pending
    // Clear all pending bits
    int32_t pending = CORE0_MBOX0_CLR;
    CORE0_MBOX0_CLR = pending;

    return IRQ_RESCHED_SGI;
  }

  return IRQ_NONE;
}

void irq_controller_eoi(uint32_t irq) {
  (void)irq;
  // No explicit EOI needed for local peripheral block
}

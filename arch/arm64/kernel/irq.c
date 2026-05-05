#include "kernel/irq.h"

#include <stddef.h>
#include <stdint.h>

#include "irq_controller.h"
#include "kernel/string.h"
#include "timer.h"

#define MAX_IRQ 128

static irq_handler_t irq_table[MAX_IRQ];

void irq_ack(uint32_t irq) { irq_controller_eoi(irq); }

void irq_disable(void) { asm volatile("msr daifset, #2" ::: "memory"); }

void irq_enable(void) { asm volatile("msr daifclr, #2" ::: "memory"); }

uint32_t irq_get_pending(void) {
  uint32_t irq = irq_controller_ack();

  if (irq == 1023) {
    return IRQ_NONE;
  }

  return irq;
}

void irq_handler(void) {
  uint32_t irq = irq_get_pending();
  if (irq == IRQ_NONE) {
    return;
  }

  if (irq < MAX_IRQ && irq_table[irq]) {
    irq_table[irq](NULL);
  }

  irq_ack(irq);
}

void irq_init(void) {
  memset(irq_table, 0, sizeof(irq_table));

  irq_controller_init();
  timer_init();
}

void register_irq(uint32_t irq, irq_handler_t handler) {
  if (irq < MAX_IRQ) {
    irq_table[irq] = handler;
  }
}

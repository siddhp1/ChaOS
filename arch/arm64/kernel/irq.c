#include "kernel/irq.h"

#include <stddef.h>
#include <stdint.h>

#include "gic.h"
#include "irq_internal.h"
#include "timer.h"

#define MAX_IRQ 128

static irq_handler_t irq_table[MAX_IRQ];

void irq_ack(uint32_t irq) { gic_eoi(irq); }

void irq_disable(void) { asm volatile("msr daifset, #2" ::: "memory"); }

void irq_enable(void) { asm volatile("msr daifclr, #2" ::: "memory"); }

uint32_t irq_get_pending(void) {
  uint32_t irq = gic_ack();

  if (irq == 1023) {
    return -1;
  }

  return irq;
}

void irq_handler(void) {
  uint32_t irq = irq_get_pending();

  if (irq < MAX_IRQ && irq_table[irq]) {
    irq_table[irq](NULL);
  }

  irq_ack(irq);
}

void irq_init(void) {
  for (int i = 0; i < MAX_IRQ; i++) {
    irq_table[i] = 0;
  }

  gic_init();
  timer_init();
  irq_enable();
}

void register_irq(int irq, irq_handler_t handler) {
  if (irq < MAX_IRQ) {
    irq_table[irq] = handler;
  }
}

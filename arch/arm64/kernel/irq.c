#include "kernel/irq.h"

#include <stddef.h>
#include <stdint.h>

#include "gic.h"
#include "timer.h"

#define MAX_IRQ 128

static irq_handler_t irq_table[MAX_IRQ];

void irq_ack(int32_t irq) { gic_eoi(irq); }

void irq_disable(void) { asm volatile("msr daifset, #2" ::: "memory"); }

void irq_enable(void) { asm volatile("msr daifclr, #2" ::: "memory"); }

int32_t irq_get_pending(void) {
  int32_t irq = gic_ack();

  if (irq == 1023) {
    return -1;
  }

  return irq;
}

void irq_handler(void) {
  int32_t irq = irq_get_pending();
  if (irq < 0) {
    return;
  }

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
}

void register_irq(int32_t irq, irq_handler_t handler) {
  if (irq < MAX_IRQ) {
    irq_table[irq] = handler;
  }
}

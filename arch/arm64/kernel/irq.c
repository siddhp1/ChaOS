#include <asm/gic.h>
#include <asm/irq.h>
#include <asm/timer.h>

void irq_init(void) {
  gic_init();
  timer_init();

  // Enable IRQ
  asm volatile("msr daifclr, #2" ::: "memory");
}

void irq_handler(void) {
  uint32_t irq = gic_ack();

  if (irq == 1023) return;

  if (irq == 27) timer_interrupt();

  gic_eoi(irq);
}

#ifndef IRQ_INTERNAL_H
#define IRQ_INTERNAL_H

#include <stdint.h>

typedef void (*irq_handler_t)(void*);

void irq_ack(uint32_t irq);
uint32_t irq_get_pending(void);
void irq_handler(void);
void register_irq(int irq, irq_handler_t handler);

#endif

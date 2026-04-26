#ifndef IRQ_H
#define IRQ_H

#include <stdint.h>

typedef void (*irq_handler_t)(void*);

void irq_ack(int32_t irq);
int32_t irq_get_pending(void);
void irq_handler(void);
void register_irq(int32_t irq, irq_handler_t handler);

void irq_disable(void);
void irq_enable(void);
void irq_init(void);

#endif

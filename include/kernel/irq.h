#ifndef IRQ_H
#define IRQ_H

#include <stdint.h>

#define IRQ_RESCHED_SGI 0
#define IRQ_TIMER 27
#define IRQ_UART 33
#define IRQ_NONE 0xFFFFFFFFUL

typedef void (*irq_handler_t)(void*);

void irq_ack(uint32_t irq);
uint32_t irq_get_pending(void);
void irq_handler(void);
void register_irq(uint32_t irq, irq_handler_t handler);

void irq_disable(void);
void irq_enable(void);

uint64_t irq_save(void);
void irq_restore(uint64_t daif);

void irq_init(void);
void irq_send_resched(void);

#endif

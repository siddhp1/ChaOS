#ifndef GIC_H
#define GIC_H

#include <stdint.h>

#define IRQ_RESCHED_SGI 0

void gic_init(void);
uint32_t gic_ack(void);
void gic_eoi(uint32_t irq);
void gic_send_sgi(uint64_t sgi_id);

#endif

#ifndef IRQ_CONTROLLER
#define IRQ_CONTROLLER

#include <stdint.h>

void irq_controller_init(void);
uint32_t irq_controller_ack(void);
void irq_controller_eoi(uint32_t irq);

#endif

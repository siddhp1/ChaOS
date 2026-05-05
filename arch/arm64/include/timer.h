#ifndef TIMER_H
#define TIMER_H

// TODO: Increase frequency
#define TIMER_HZ 10
#define TIMER_IRQ 27

void timer_init(void);
void timer_interrupt(void* unused);

#endif

#ifndef EXCEPTION_H
#define EXCEPTION_H

void panic_sync(void);
void panic_irq(void);
void panic_fiq(void);
void panic_serr(void);

#endif

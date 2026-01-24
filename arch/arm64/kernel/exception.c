#include "kernel/exception.h"

#include "kernel/panic.h"

void panic_sync(void) { panic("Synchronous exception"); }

void panic_irq(void) { panic("IRQ exception"); }

void panic_fiq(void) { panic("FIQ exception"); }

void panic_serr(void) { panic("SError exception"); }

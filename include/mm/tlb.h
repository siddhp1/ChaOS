#ifndef TLB_H
#define TLB_H

#include <stdint.h>

void tlb_flush_addr(uint64_t va);
void tlb_flush_all(void);

#endif

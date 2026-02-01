#ifndef TLB_H
#define TLB_H

static inline void tlb_flush_all(void) {
  asm volatile(
      "dsb ish\n"
      "tlbi vmalle1\n"
      "dsb ish\n"
      "isb\n" ::
          : "memory");
}

#endif

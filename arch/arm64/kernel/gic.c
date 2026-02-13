#include "gic.h"

#include <stdint.h>

#define GICD_BASE 0x08000000UL
#define GICR_BASE 0x080A0000UL

#define GICR_RD_BASE (GICR_BASE)
#define GICR_SGI_BASE (GICR_BASE + 0x10000UL)

#define GICD_CTLR 0x0000
#define GICD_CTLR_RWP (1u << 31)

#define GICR_WAKER 0x0014
#define GICR_WAKER_PROCESSOR_SLEEP (1u << 1)
#define GICR_WAKER_CHILDREN_ASLEEP (1u << 2)

#define GICR_IGROUPR0 0x0080
#define GICR_ISENABLER0 0x0100
#define GICR_IPRIORITYR 0x0400

void gic_init(void) {
  uint64_t sre;
  asm volatile("mrs %0, ICC_SRE_EL1" : "=r"(sre));
  sre |= 1u;
  asm volatile("msr ICC_SRE_EL1, %0" : : "r"(sre) : "memory");
  asm volatile("isb" ::: "memory");

  asm volatile("msr ICC_PMR_EL1, %0" : : "r"((uint64_t)0xff) : "memory");
  asm volatile("msr ICC_IGRPEN1_EL1, %0" : : "r"((uint64_t)1) : "memory");
  asm volatile("isb" ::: "memory");

  *(volatile uint32_t*)(GICD_BASE + GICD_CTLR) = 0x3;

  while ((*(volatile uint32_t*)(GICD_BASE + GICD_CTLR)) & GICD_CTLR_RWP);

  // Wake redistributor (RD frame)
  uint32_t waker = *(volatile uint32_t*)(GICR_RD_BASE + GICR_WAKER);
  waker &= ~GICR_WAKER_PROCESSOR_SLEEP;
  *(volatile uint32_t*)(GICR_RD_BASE + GICR_WAKER) = waker;
  while ((*(volatile uint32_t*)(GICR_RD_BASE + GICR_WAKER)) &
         GICR_WAKER_CHILDREN_ASLEEP);

  // Put PPI 27 (CNTVIRQ) into Group1 so ICC_IAR1_EL1 sees it (SGI/PPI frame)
  uint32_t grp = *(volatile uint32_t*)(GICR_SGI_BASE + GICR_IGROUPR0);
  grp |= (1u << 27);
  *(volatile uint32_t*)(GICR_SGI_BASE + GICR_IGROUPR0) = grp;

  volatile uint8_t* p = (volatile uint8_t*)(GICR_SGI_BASE + GICR_IPRIORITYR);
  p[27] = 0x80;

  // Enable PPI 27 (SGI/PPI frame)
  *(volatile uint32_t*)(GICR_SGI_BASE + GICR_ISENABLER0) = (1u << 27);
}

uint32_t gic_ack(void) {
  uint64_t iar;
  asm volatile("mrs %0, ICC_IAR1_EL1" : "=r"(iar));
  return (uint32_t)iar;
}

void gic_eoi(uint32_t irq) {
  asm volatile("msr ICC_EOIR1_EL1, %0" : : "r"((uint64_t)irq) : "memory");
}

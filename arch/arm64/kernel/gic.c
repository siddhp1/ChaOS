#include "gic.h"

#include <stdint.h>

#include "mm/kmap.h"

#define GICD_BASE (KERNEL_VIRT_BASE + 0x08000000UL)
#define GICR_BASE (KERNEL_VIRT_BASE + 0x080A0000UL)
#define GICR_RD_BASE (GICR_BASE)

// Software-generated interrupt base
#define GICR_SGI_BASE (GICR_BASE + 0x10000UL)

#define GICD_CTLR 0x0000          // Distributor control register offset
#define GICD_CTLR_RWP (1u << 31)  // Register write pending mask

#define GICR_WAKER 0x0014  // Redistributor wake register offset
#define GICR_WAKER_PROCESSOR_SLEEP (1u << 1)
#define GICR_WAKER_CHILDREN_ASLEEP (1u << 2)

#define GICR_IGROUPR0 0x0080    // Interrupt group register 0 offset
#define GICR_ISENABLER0 0x0100  // Interrupt set enable register 0 offset
#define GICR_IPRIORITYR 0x0400  // Interrupt priority register offset

void gic_init(void) {
  // Enable the system register interface to the GIC CPU interface for EL1
  uint64_t sre;
  asm volatile("mrs %0, ICC_SRE_EL1" : "=r"(sre));
  sre |= 1u;
  asm volatile("msr ICC_SRE_EL1, %0" : : "r"(sre) : "memory");
  asm volatile("isb" ::: "memory");

  // Allow all interrupt priorities through
  asm volatile("msr ICC_PMR_EL1, %0" : : "r"((uint64_t)0xff) : "memory");

  // Enable group 1 interrupts
  asm volatile("msr ICC_IGRPEN1_EL1, %0" : : "r"((uint64_t)1) : "memory");
  asm volatile("isb" ::: "memory");

  // Enable distributor for group 0 and group 1
  *(volatile uint32_t*)(GICD_BASE + GICD_CTLR) = 0x3;
  while ((*(volatile uint32_t*)(GICD_BASE + GICD_CTLR)) & GICD_CTLR_RWP);

  // Wake redistributor
  uint32_t waker = *(volatile uint32_t*)(GICR_RD_BASE + GICR_WAKER);
  waker &= ~GICR_WAKER_PROCESSOR_SLEEP;
  *(volatile uint32_t*)(GICR_RD_BASE + GICR_WAKER) = waker;
  while ((*(volatile uint32_t*)(GICR_RD_BASE + GICR_WAKER)) &
         GICR_WAKER_CHILDREN_ASLEEP);

  // Put PPI 27 (CNTVIRQ) into group 1
  uint32_t grp = *(volatile uint32_t*)(GICR_SGI_BASE + GICR_IGROUPR0);
  grp |= (1u << 27);
  *(volatile uint32_t*)(GICR_SGI_BASE + GICR_IGROUPR0) = grp;

  // Set priority for PPI 27
  volatile uint8_t* p = (volatile uint8_t*)(GICR_SGI_BASE + GICR_IPRIORITYR);
  p[27] = 0x80;

  // Enable PPI 27
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

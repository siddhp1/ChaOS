#include "irq_controller.h"

#include <stdbool.h>
#include <stdint.h>

#include "kernel/irq.h"
#include "mm/kmap.h"

#define GICD_BASE (KERNEL_VIRT_BASE + 0x08000000UL)
#define GICR_BASE (KERNEL_VIRT_BASE + 0x080A0000UL)

// Software-generated interrupt base
#define GICR_SGI_BASE (GICR_BASE + 0x10000UL)

#define GICD_CTLR 0x0000          // Distributor control register offset
#define GICD_CTLR_RWP (1u << 31)  // Register write pending mask
#define GICD_ICFGR 0x0C00         // Interrupt configuration register offset
#define GICD_IROUTER 0x6000       // Interrupt router register offset

#define GICD_REG_SIZE 4
#define GICD_IROUTER_SIZE 8

#define GICR_WAKER 0x0014  // Redistributor wake register offset
#define GICR_WAKER_PROCESSOR_SLEEP (1u << 1)
#define GICR_WAKER_CHILDREN_ASLEEP (1u << 2)

#define GIC_IGROUPR 0x0080     // Interrupt group register 0 offset
#define GIC_ISENABLER 0x0100   // Interrupt set enable register 0 offset
#define GIC_IPRIORITYR 0x0400  // Interrupt priority register offset

static void gicr_config(uint32_t int_id, uint8_t prio, bool group1) {
  if (int_id > 31) {
    return;
  }

  volatile uint32_t* group = (volatile uint32_t*)(GICR_SGI_BASE + GIC_IGROUPR);
  volatile uint8_t* priority =
      (volatile uint8_t*)(GICR_SGI_BASE + GIC_IPRIORITYR);
  volatile uint32_t* enable =
      (volatile uint32_t*)(GICR_SGI_BASE + GIC_ISENABLER);

  uint32_t mask = 1u << int_id;

  if (group1) {
    *group |= mask;
  } else {
    *group &= ~mask;
  }

  priority[int_id] = prio;
  *enable = mask;
}

static void gicd_config(uint32_t int_id, uint8_t prio, bool group1) {
  if (int_id < 32) {
    return;
  }

  uint32_t reg_idx = int_id / 32;  // 32 interrupts per register
  uint32_t bit = int_id % 32;
  uint32_t mask = 1u << bit;

  volatile uint32_t* group =
      (volatile uint32_t*)(GICD_BASE + GIC_IGROUPR + reg_idx * GICD_REG_SIZE);
  volatile uint32_t* enable =
      (volatile uint32_t*)(GICD_BASE + GIC_ISENABLER + reg_idx * GICD_REG_SIZE);
  volatile uint8_t* priority = (volatile uint8_t*)(GICD_BASE + GIC_IPRIORITYR);
  volatile uint32_t* icfgr =
      (volatile uint32_t*)(GICD_BASE + GICD_ICFGR +
                           (int_id / 16) * GICD_REG_SIZE);  // 16 interrupts per
                                                            // config register
  volatile uint64_t* irouter =
      (volatile uint64_t*)(GICD_BASE + GICD_IROUTER +
                           ((uint64_t)int_id * GICD_IROUTER_SIZE));

  if (group1) {
    *group |= mask;
  } else {
    *group &= ~mask;
  }

  priority[int_id] = prio;

  // Level-triggered, active low
  uint32_t shift = (int_id % 16) * 2;
  uint32_t cfg = *icfgr;
  cfg &= ~(3u << shift);
  *icfgr = cfg;

  // Route to Core 0
  *irouter = 0;

  *enable = mask;
}

void irq_controller_init(void) {
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
  uint32_t waker = *(volatile uint32_t*)(GICR_BASE + GICR_WAKER);
  waker &= ~GICR_WAKER_PROCESSOR_SLEEP;
  *(volatile uint32_t*)(GICR_BASE + GICR_WAKER) = waker;
  while ((*(volatile uint32_t*)(GICR_BASE + GICR_WAKER)) &
         GICR_WAKER_CHILDREN_ASLEEP);

  gicr_config(IRQ_TIMER, 0x80, true);
  gicr_config(IRQ_RESCHED_SGI, 0x80, true);
  gicd_config(IRQ_UART, 0x80, true);

  asm volatile("dsb sy" ::: "memory");
  asm volatile("isb" ::: "memory");
}

void irq_controller_send_sgi(uint64_t sgi_id) {
  uint8_t target = 1u;  // Target Core 0
  uint64_t val = (target & 0xffffu) | ((sgi_id & 0xf) << 24);
  asm volatile("dsb sy" ::: "memory");
  asm volatile("msr ICC_SGI1R_EL1, %0" : : "r"(val) : "memory");
  asm volatile("isb" ::: "memory");
}

uint32_t irq_controller_ack(void) {
  uint64_t iar;
  asm volatile("mrs %0, ICC_IAR1_EL1" : "=r"(iar));
  return (uint32_t)iar;
}

void irq_controller_eoi(uint32_t irq) {
  asm volatile("msr ICC_EOIR1_EL1, %0" : : "r"((uint64_t)irq) : "memory");
}

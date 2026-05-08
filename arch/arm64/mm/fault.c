#include "fault.h"

#include <stdint.h>

#include "kernel/initramfs.h"
#include "kernel/irq_frame.h"
#include "kernel/panic.h"
#include "kernel/printk.h"
#include "kernel/scheduler/scheduler.h"
#include "kernel/task.h"

static bool from_el0(uint64_t spsr) { return (spsr & 0xF) == SPSR_EL0; }

static const char* access_type_from_esr(uint64_t esr) {
  return (esr & (1ULL << 6)) ? "write" : "read";
}

static const char* reason_from_esr(uint64_t esr) {
  uint64_t dfsc = esr & 0x3F;
  if ((dfsc & 0x3C) == 0x04) return "translation fault";
  if ((dfsc & 0x3C) == 0x0C) return "permission fault";
  return "fault";
}

static void kill_user_task(const char* reason, uint64_t pc, uint64_t far,
                           const char* access_type) {
  if (!current_task) {
    panic("User fault with no current task");
  }

  printk("Segmentation fault: pid=%d pc=%lx far=%lx %s %s\n", current_task->pid,
         pc, far, access_type, reason);

  task_exit(current_task, 1);
}

void data_abort_handler(void) {
  uint64_t elr, esr, far, spsr;

  asm volatile("mrs %0, ELR_EL1" : "=r"(elr));
  asm volatile("mrs %0, ESR_EL1" : "=r"(esr));
  asm volatile("mrs %0, FAR_EL1" : "=r"(far));
  asm volatile("mrs %0, SPSR_EL1" : "=r"(spsr));

  if (from_el0(spsr)) {
    kill_user_task(reason_from_esr(esr), elr, far, access_type_from_esr(esr));
    return;
  }

  printk("DATA ABORT!\nELR_EL1=%lx\nESR_EL1=%lx\nFAR_EL1=%lx\nSPSR_EL1=%lx",
         elr, esr, far, spsr);
  panic("Kernel page fault detected");
}

void prefetch_abort_handler(void) {
  uint64_t elr, esr, far, spsr;

  asm volatile("mrs %0, ELR_EL1" : "=r"(elr));
  asm volatile("mrs %0, ESR_EL1" : "=r"(esr));
  asm volatile("mrs %0, FAR_EL1" : "=r"(far));
  asm volatile("mrs %0, SPSR_EL1" : "=r"(spsr));

  if (from_el0(spsr)) {
    kill_user_task(reason_from_esr(esr), elr, far, "exec");
    return;
  }

  printk("PREFETCH ABORT!\nELR_EL1=%lx\nESR_EL1=%lx\nFAR_EL1=%lx\nSPSR_EL1=%lx",
         elr, esr, far, spsr);
  panic("Kernel instruction fetch fault detected");
}

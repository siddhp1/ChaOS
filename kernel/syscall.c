#include "kernel/syscall.h"

#include <stddef.h>
#include <stdint.h>

#include "kernel/printk.h"
#include "kernel/uart.h"

// Syscall numbers
#define SYS_WRITE 1
#define SYS_EXIT 2

// Syscall frame (saved by el0_sync)
struct syscall_frame {
  uint64_t x0, x1, x2, x3, x4, x5, x6, x7;
  uint64_t x8, x9, x10, x11, x12, x13, x14, x15;
  uint64_t x16, x17, x18, x19, x20, x21, x22, x23;
  uint64_t x24, x25, x26, x27, x28, x29;
  uint64_t x30, _pad;
  uint64_t elr, spsr;
  uint64_t sp_el0, _pad2;
};

// sys_write(fd, buf, count)
// For now, only supports fd=1 (stdout) and ignores user buffer validation
static int64_t sys_write(int fd, const char* buf, size_t count) {
  if (fd != 1) {
    return -1;  // Only support stdout for now
  }

  // TODO: Validate user buffer is in user space
  // TODO: Copy from user space properly

  for (size_t i = 0; i < count; i++) {
    uart_putc(buf[i]);
  }

  return count;
}

// sys_exit(status)
static void sys_exit(int status) {
  printk("User process exited with status: ");
  printk_hex_u64(status);
  printk("\n");

  // TODO: Actually terminate the process
  // For now, just loop
  while (1) {
    asm volatile("wfi");
  }
}

void handle_el0_sync(struct syscall_frame* frame) {
  // Read exception syndrome register
  uint64_t esr;
  asm volatile("mrs %0, ESR_EL1" : "=r"(esr));

  uint32_t ec = (esr >> 26) & 0x3F;  // Exception class

  if (ec == 0x15) {  // SVC instruction from AArch64
    // Syscall number is in x8
    uint64_t syscall_num = frame->x8;

    // Arguments in x0-x5 (following ARM64 calling convention)
    switch (syscall_num) {
      case SYS_WRITE:
        frame->x0 = sys_write(frame->x0, (const char*)frame->x1, frame->x2);
        break;

      case SYS_EXIT:
        sys_exit(frame->x0);
        break;

      default:
        printk("Unknown syscall: ");
        printk_hex_u64(syscall_num);
        printk("\n");
        frame->x0 = -1;
        break;
    }
  } else {
    // Other synchronous exception (page fault, etc.)
    printk("EL0 sync exception, EC=");
    printk_hex_u64(ec);
    printk(" ESR=");
    printk_hex_u64(esr);
    printk("\n");

    // TODO: Switch to panic
    // For now, panic
    while (1) {
      asm volatile("wfi");
    }
  }
}

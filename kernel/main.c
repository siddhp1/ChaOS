#include <stdint.h>

#include "asm/mmu.h"
#include "kernel/cpu.h"
#include "kernel/printk.h"
#include "kernel/uart.h"
#include "memory/heap.h"
#include "memory/kmap.h"
#include "memory/memory.h"
#include "memory/page.h"

extern uintptr_t bss_start;
extern uintptr_t bss_end;

void kernel_entry(void) {
  // Zero out the BSS (static variables)
  volatile uintptr_t* ptr = (volatile uintptr_t*)&bss_start;
  while (ptr < &bss_end) {
    *ptr++ = 0;
  }

  // Phase one
  uart_init();
  exception_init();

  printk("Hello OS!\n");

  // Trigger a panic to test exception handling
  // volatile int* p = (int*)0xDEADBEEF;
  // volatile int x = *p;
  // (void)x;

  // Phase two
  memory_init();

  // Test allocating and freeing a page
  struct page* page = alloc_page();
  void* virtual = kmap(page);
  uintptr_t* vptr = (uintptr_t*)virtual;
  while ((uintptr_t)vptr < (uintptr_t) virtual + PAGE_SIZE) {
    *vptr++ = 0xAB;
  }
  free_page(page);

  while (1) asm volatile("WFI");
}

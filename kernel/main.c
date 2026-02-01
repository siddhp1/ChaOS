#include <stdint.h>

#include "kernel/cpu.h"
#include "kernel/printk.h"
#include "kernel/uart.h"
#include "mm/heap.h"
#include "mm/kmap.h"
#include "mm/memory.h"
#include "mm/mmu.h"
#include "mm/page.h"

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
  printk("UART initialized\n");

  exception_init();
  printk("Exceptions initialized\n");

  // Trigger a panic to test exception handling
  // volatile int* p = (int*)0xDEADBEEF;
  // volatile int x = *p;
  // (void)x;

  // Phase two
  memory_init();
  printk("Memory initialized\n");

  // Test allocating and freeing a page
  struct page* page = alloc_page();
  void* virtual = kmap(page);
  dump_free_pages();

  uintptr_t* vptr = (uintptr_t*)virtual;
  while ((uintptr_t)vptr < (uintptr_t) virtual + PAGE_SIZE) {
    *vptr++ = 0xAB;
  }
  dump_page(page);

  free_page(page);
  dump_free_pages();

  printk("Tests complete\n");
  while (1) asm volatile("WFI");
}

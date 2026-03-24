#include "mm/kvmalloc.h"

#include <stddef.h>
#include <stdint.h>

#include "mm/kmap.h"
#include "mm/mmu.h"
#include "mm/page.h"
#include "mm/pgtable.h"

// Virtual address space for dynamic kernel allocations
// Start after the direct-mapped region (after 2 GiB of RAM)
#define KVMALLOC_BASE (KERNEL_VIRT_BASE + 0x80000000UL)  // + 2 GiB
#define KVMALLOC_SIZE (0x40000000UL)                     // 1 GiB

static uint64_t next_kvirt = KVMALLOC_BASE;

void* kvmalloc(size_t size) {
  if (size == 0 || size > PAGE_SIZE) {
    return NULL;  // Only support single page for now
  }

  struct page* p = alloc_page();
  if (!p) {
    return NULL;
  }

  uint64_t phys = page_to_phys(p);
  uint64_t virt = next_kvirt;
  next_kvirt += PAGE_SIZE;

  if (next_kvirt >= KVMALLOC_BASE + KVMALLOC_SIZE) {
    free_page(p);
    return NULL;
  }

  uint64_t attrs = PTE_AF | PTE_SH_INNER | PTE_ATTRINDX(1) |  // Normal memory
                   PTE_UXN | PTE_PXN;                         // No execute

  uint64_t* l0 = get_kernel_l0_table();
  if (map_page_l3(l0, virt, phys, attrs) != 0) {
    free_page(p);
    return NULL;
  }

  return (void*)virt;
}

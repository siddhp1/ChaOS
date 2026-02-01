#include "mm/memory.h"

#include "mm/mmu.h"
#include "mm/page.h"
#include "page_internal.h"
#include "phys.h"

void memory_init(void) {
  mmu_init();
  phys_init();
  page_init();
}

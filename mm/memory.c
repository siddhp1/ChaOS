#include "mm/memory.h"

#include "mm/page.h"
#include "page_internal.h"
#include "phys.h"

void memory_init(void) {
  phys_init();
  page_init();
}

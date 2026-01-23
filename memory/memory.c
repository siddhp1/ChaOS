#include "memory/memory.h"

#include "memory/page.h"
#include "memory/physical.h"

void memory_init(void) {
  phys_init();
  page_init();
}

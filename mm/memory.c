#include "mm/memory.h"

#include "mm/page.h"
#include "mm/slab.h"

void memory_init(void) {
  page_init();
  slab_init();
}

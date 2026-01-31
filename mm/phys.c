#include "mm/phys.h"

uintptr_t phys_start;
uintptr_t phys_end;

void phys_init(void) {
  phys_start = PHYS_START;
  phys_end = PHYS_END;
}

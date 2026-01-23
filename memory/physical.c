#include "memory/physical.h"

uintptr_t physical_start;
uintptr_t physical_end;

void phys_init(void) {
  physical_start = PHYSICAL_START;
  physical_end = PHYSICAL_END;
}

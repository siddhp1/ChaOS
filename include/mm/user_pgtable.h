#ifndef USER_PGTABLE_H
#define USER_PGTABLE_H

#include <stdint.h>

void free_user_pgd(uintptr_t pgd_phys);

#endif

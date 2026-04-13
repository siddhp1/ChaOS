#ifndef USER_PGTABLE_H
#define USER_PGTABLE_H

#include <stdint.h>

void free_user_pgd(uintptr_t pgd_phys);
int map_user_page(uint64_t* l0_table_phys, uint64_t va, uint64_t phys,
                  uint64_t attrs);

#endif

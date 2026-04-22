#ifndef USER_PGTABLE_H
#define USER_PGTABLE_H

#include <stdint.h>

uintptr_t copy_user_pgd(uint64_t* src_pgd);
void free_page_table(uintptr_t table_phys, uint8_t level);
void free_user_pgd(uintptr_t pgd_phys);
int map_user_page(uint64_t* l0_table_phys, uint64_t va, uint64_t phys,
                  uint64_t attrs);

#endif

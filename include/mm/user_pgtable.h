#ifndef USER_PGTABLE_H
#define USER_PGTABLE_H

#include <stdint.h>

// Allocate a new user page directory (L0 table)
uint64_t* alloc_user_pgd(void);

// Free all user page tables
void free_user_pgd(uint64_t* pgd_phys);

#endif

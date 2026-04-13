#ifndef PGTABLE_H
#define PGTABLE_H

#include <stdint.h>

#define NUM_TABLE_ENTRIES 512

#define PTE_VALID (1UL << 0)
#define PTE_TABLE (1UL << 1)
#define PTE_USER (1UL << 6)
#define PTE_PAGE (3UL << 0)
#define PTE_AF (1UL << 10)
#define PTE_SH_INNER (3UL << 8)
#define PTE_ATTRINDX(idx) ((idx) << 2)
#define PTE_PXN (1ULL << 53)
#define PTE_UXN (1ULL << 54)

#define PTE_TYPE_MASK 0x3
#define PTE_TYPE_TABLE 0x3
#define PTE_IS_TABLE(pte) (((pte) & PTE_TYPE_MASK) == PTE_TYPE_TABLE)

#define PTE_NLTA_MASK 0x0000FFFFFFFFF000ULL
#define PTE_NLTA(pte) ((pte) & PTE_NLTA_MASK)

uint64_t* get_kernel_l0_table(void);

uintptr_t setup_identity_tables(void);
uintptr_t setup_higher_half_tables(void);

uintptr_t alloc_page_table(void);

int map_page_l3(uint64_t* l0_table, uint64_t va, uint64_t phys, uint64_t attrs);

uint64_t* copy_user_pgd(uint64_t* src_pgd);

#endif

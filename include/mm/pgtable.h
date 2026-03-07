#ifndef PGTABLE_H
#define PGTABLE_H

#include <stdint.h>

#define PAGE_SHIFT 12
#define PAGE_SIZE (1UL << PAGE_SHIFT)
#define PTRS_PER_TABLE 512

#define PTE_VALID ((uint64_t)1 << 0)
#define PTE_TABLE ((uint64_t)1 << 1)
#define PTE_PAGE (3UL << 0)  // For L3: bits [1:0] = 11
#define PTE_AF ((uint64_t)1 << 10)
#define PTE_SH_INNER ((uint64_t)3 << 8)
#define PTE_ATTRINDX(idx) ((uint64_t)(idx) << 2)
#define PTE_PXN ((uint64_t)1 << 53)
#define PTE_UXN ((uint64_t)1 << 54)

int map_page_l3(uint64_t* l0_table, uint64_t va, uint64_t phys, uint64_t attrs);

#endif

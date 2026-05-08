#include "kernel/user_access.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "kernel/scheduler/scheduler.h"
#include "kernel/string.h"
#include "kernel/user_thread.h"
#include "mm/kmap.h"
#include "mm/pgtable.h"
#include "mm/user_pgtable.h"

static uint64_t* lookup_l3_pte(uint64_t* l0_table, uint64_t va) {
  uint64_t l0e = l0_table[L0_INDEX(va)];
  if (!PTE_IS_VALID(l0e) || !PTE_IS_TABLE(l0e)) return NULL;

  uint64_t* l1 = (uint64_t*)(KERNEL_VIRT_BASE + PTE_NLTA(l0e));
  uint64_t l1e = l1[L1_INDEX(va)];
  if (!PTE_IS_VALID(l1e) || !PTE_IS_TABLE(l1e)) return NULL;

  uint64_t* l2 = (uint64_t*)(KERNEL_VIRT_BASE + PTE_NLTA(l1e));
  uint64_t l2e = l2[L2_INDEX(va)];
  if (!PTE_IS_VALID(l2e) || !PTE_IS_TABLE(l2e)) return NULL;

  uint64_t* l3 = (uint64_t*)(KERNEL_VIRT_BASE + PTE_NLTA(l2e));
  return &l3[L3_INDEX(va)];
}

bool user_va_mapped(uintptr_t va, size_t len) {
  if (!current_task || !current_task->ttbr0) return false;
  if (len == 0) return true;

  uintptr_t start = va & ~(PAGE_SIZE - 1);
  uintptr_t end = (va + len - 1) & ~(PAGE_SIZE - 1);

  uint64_t* l0 = (uint64_t*)(KERNEL_VIRT_BASE + current_task->ttbr0);

  for (uintptr_t addr = start; addr <= end; addr += PAGE_SIZE) {
    uint64_t* pte = lookup_l3_pte(l0, addr);
    if (!pte) return false;

    uint64_t entry = *pte;
    if (!PTE_IS_VALID(entry)) return false;
    if (!(entry & PTE_USER)) return false;
    if (!(entry & PTE_AF)) return false;
  }

  return true;
}

bool user_range_ok(uintptr_t addr, uint64_t len) {
  if (len == 0) return true;
  if (addr < USER_VIRT_START) return false;
  if (addr >= USER_VIRT_END) return false;
  if (len > (USER_VIRT_END - addr)) return false;
  return true;
}

long copy_from_user(void* dst, const void* src, uint64_t len) {
  if (!user_range_ok((uintptr_t)src, len)) return -1;

  if (!user_va_mapped((uintptr_t)src, len)) return -1;

  memcpy(dst, src, len);

  return 0;
}

long copy_to_user(void* dst, const void* src, uint64_t len) {
  if (!user_range_ok((uintptr_t)dst, len)) return -1;

  if (!user_va_mapped((uintptr_t)dst, len)) return -1;

  memcpy(dst, src, len);

  return 0;
}

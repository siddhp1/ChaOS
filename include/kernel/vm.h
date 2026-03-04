#ifndef VM_H
#define VM_H

#include <stdint.h>

#include "mm/page.h"

struct task;

#define USER_BASE 0x00400000UL
#define USER_STACK_TOP 0x00800000UL

#define VM_USER_RW (1UL << 6)  // User accessible
#define VM_USER_EXEC (0UL)     // Executable
#define VM_USER_RWX (VM_USER_RW | VM_USER_EXEC)

uint64_t vm_create_user_ttbr0(void);

void vm_map_user_page(uint64_t ttbr0, uint64_t va, struct page *page,
                      uint64_t flags);

void vm_destroy_user(struct task *t);

#endif

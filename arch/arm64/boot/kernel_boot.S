.section .text.boot
.global _start
.extern kernel_entry
.extern mmu_init

_start:
    adrp x0, boot_stack_top // Copy top 52 bits
    add  x0, x0, :lo12:boot_stack_top // Copy bottom 12 bits
    mov  sp, x0

    // Disable FP/NEON
    mrs  x0, CPACR_EL1
    bic  x0, x0, #(3 << 20)
    msr  CPACR_EL1, x0
    isb

    // Zero BSS
    ldr  x0, =bss_start_phys
    ldr  x1, =bss_end_phys
1:  cmp  x0, x1
    b.ge 2f
    str  xzr, [x0], #8
    b    1b
2:

    // Compute the physical address of mmu_init
    ldr  x0, =mmu_init
    ldr  x1, =KERNEL_VIRT_OFFSET
    sub  x0, x0, x1
    blr  x0 // Jump to mmu_init

    ldr  x0, =_higher_half_entry
    br   x0

.section .text
.global _higher_half_entry

_higher_half_entry:
    ldr  x0, =stack_top
    mov  sp, x0

    // Zero TTBR0 (identity mapping)
    msr  TTBR0_EL1, xzr

    // TLB flush
    isb
    tlbi vmalle1
    dsb  sy
    isb

    bl   kernel_entry

3:  wfi
    b    3b

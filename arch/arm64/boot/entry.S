.section .text.boot
.global _start
.extern stack_top
.extern kernel_entry
.extern mmu_init
.extern bss_start_phys
.extern bss_end_phys

_start:
    adrp x0, boot_stack_top
    add  x0, x0, :lo12:boot_stack_top // page offset
    mov  sp, x0

    mrs x0, CPACR_EL1
    bic x0, x0, #(3 << 20)
    msr CPACR_EL1, x0
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
    blr  x0

    ldr  x0, =_higher_half_entry
    br   x0

.section .text
.global _higher_half_entry
_higher_half_entry:
    ldr  x0, =stack_top
    mov  sp, x0

    // TODO: Consider removing this
    // Disable TTBR0 (identity mapping)
    msr  TTBR0_EL1, xzr
    isb
    tlbi vmalle1
    dsb  sy
    isb

    bl   kernel_entry

3:  wfi
    b    3b

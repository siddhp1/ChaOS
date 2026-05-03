.section .text.boot
.global _start
.extern kernel_entry
.extern mmu_init

_start:
    // Park secondary cores
    mrs x0, MPIDR_EL1
    and x0, x0, #0xFF // Aff0 (core ID within cluster)
    cbz x0, 2f
1:
    wfe
    b 1b

2:
    adrp x0, boot_stack_top // Copy top 52 bits
    add x0, x0, :lo12:boot_stack_top // Copy bottom 12 bits
    mov sp, x0

    // Detect EL
    mrs x0, CurrentEL
    lsr x0, x0, #2
    and x0, x0, #3

    cmp x0, #2
    b.eq el2_start

    cmp x0, #1
    b.eq el1_start

3:
    wfi
    b 3b

el2_start:
    msr daifset, #0xF // Mask D,A,I,F

    // Set EL1 execution state to AArch64
    mrs x0, HCR_EL2
    orr x0, x0, #(1<<31)
    msr HCR_EL2, x0
    isb

    // Give EL1 access to generic timer
    mrs x0, CNTHCTL_EL2
    orr x0, x0, #3
    msr CNTHCTL_EL2, x0
    msr CNTVOFF_EL2, xzr
    isb

    // Set EL1 stack pointer
    mov x0, sp
    msr SP_EL1, x0

    ldr x0, =el1_start
    msr ELR_EL2, x0

    mov x0, #0x3C5 // EL1h, set E,A,I,F
    msr SPSR_EL2, x0

    eret

el1_start:
    msr daifset, #2 // Mask I

    // Disable FP/NEON
    mrs x0, CPACR_EL1
    bic x0, x0, #(3<<20)
    msr CPACR_EL1, x0
    isb

    // Zero BSS
    ldr x0, =bss_start_phys
    ldr x1, =bss_end_phys
4:
    cmp x0, x1
    b.ge 5f
    str xzr, [x0], #8
    b 4b

5:
    // Compute the physical address of mmu_init
    ldr x0, =mmu_init
    ldr x1, =KERNEL_VIRT_OFFSET
    sub x0, x0, x1
    blr x0 // Jump to mmu_init

    ldr x0, =_higher_half_entry
    br x0

.section .text
.global _higher_half_entry

_higher_half_entry:
    ldr x0, =stack_top
    mov sp, x0

    // Zero TTBR0 (identity mapping)
    msr TTBR0_EL1, xzr

    // TLB flush
    isb
    tlbi vmalle1
    dsb sy
    isb

    bl kernel_entry

6:
    wfi
    b 6b

ENTRY(_start)

KERNEL_PHYS_BASE = 0x40000000;
KERNEL_VIRT_BASE = 0xFFFF000040000000;
KERNEL_VIRT_OFFSET = KERNEL_VIRT_BASE - KERNEL_PHYS_BASE;

SECTIONS {
    . = KERNEL_PHYS_BASE;

    .text.boot : {
        *(.text.boot)
    }

    . = ALIGN(16);
    . = . + 0x4000;
    boot_stack_top = .;

    . += KERNEL_VIRT_OFFSET;

    .text : AT(ADDR(.text) - KERNEL_VIRT_OFFSET) {
        *(.text*)
    }

    .rodata : AT(ADDR(.rodata) - KERNEL_VIRT_OFFSET) {
        *(.rodata*)
    }

    .data : AT(ADDR(.data) - KERNEL_VIRT_OFFSET) {
        *(.data*)
    }

    .bss (NOLOAD) : AT(ADDR(.bss) - KERNEL_VIRT_OFFSET) {
        bss_start = .;
        *(.bss*)
        *(COMMON)
        bss_end = .;
    }

    bss_start_phys = bss_start - KERNEL_VIRT_OFFSET;
    bss_end_phys = bss_end - KERNEL_VIRT_OFFSET;

    . = ALIGN(4096);
    .initramfs : AT(ADDR(.initramfs) - KERNEL_VIRT_OFFSET) {
        initramfs_start = .;
        KEEP(*(.initramfs))
        initramfs_end = .;
    }

    . = ALIGN(16);
    . = . + 0x4000;
    stack_top = .;

    . = ALIGN(4096);
    page_start = .;
}

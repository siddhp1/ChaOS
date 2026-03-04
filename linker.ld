ENTRY(_start)

SECTIONS {
    . = 0x40000000;

    .text : {
        *(.text.boot)
        *(.text*)
    }

    .rodata : {
        *(.rodata*)
    }

    .data : {
        *(.data*)
    }

    .bss (NOLOAD) : {
        bss_start = .;
        *(.bss*)
        *(COMMON)
        bss_end = .;
    }

    . = ALIGN(4096);
    .initramfs : {
        initramfs_start = .;
        KEEP(*(.initramfs))
        initramfs_end = .;
    }

    . = ALIGN(16);
    stack_top = . + 0x4000;
}

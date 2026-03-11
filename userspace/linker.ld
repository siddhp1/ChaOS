ENTRY(_start)

SECTIONS {
    . = 0x1000;  /* User programs start at 4K */

    .text : {
        KEEP(*(.text.entry))
        *(.text*)
    }

    .rodata : {
        *(.rodata*)
    }

    .data : {
        *(.data*)
    }

    .bss : {
        *(.bss*)
    }
}

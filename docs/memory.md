# Memory Management

## Heap

Kmalloc currently allocates physical pages for the kernel.
A proper allocator needs to be implemented.

## Kmap

Kmap takes a physical page and returns the kernel virtual address for that page.

## Physical

Currently a hard-coded physical RAM window for the page allocator.
For QEMU virt: begins at 0x40000000 and ends at 0x60000000 (0.5 GiB).

Initialization will be modifed when the range is runtime-discovered (e.g., device tree, firmware).

## Page

Allocate/free a page descriptor from the free list.
The free list is a singly-linked list of free pages (page internal structs).

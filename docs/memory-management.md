# Memory Management {#memory_management}

Memory initialization constructs the physical page free list, then initializes
the slab caches used by the kernel heap.

## Physical pages

ChaOS manages physical memory in 4 KiB pages. QEMU uses the half-open range
`[0x40000000, 0x60000000)`; Raspberry Pi currently uses the fixed range
`[0x00080000, 0x20000000)`.

A `page_internal` entry represents every page. During `page_init`, pages before
the linker-defined `page_start` are reserved and the rest are placed on a free
list. `alloc_page` removes the head and `free_page` returns it; both operations
save and mask IRQ state. Address conversion uses the platform physical base.

@image html free_list.svg "Page metadata and free list"

This diagram remains accurate at a conceptual level: ChaOS has a metadata entry
for every physical page and links currently available entries through
`page_internal::next`. The implementation now also marks every page before
`page_start` as reserved and masks IRQs while changing the list.

<!-- DOC-GAP(detail): Document allocator invariants, ownership, out-of-memory
     behavior, and the intended meaning of flags/refcount. The implementation
     does not currently enforce double-free or reference counting. -->

Every mapped physical page is reachable through the higher-half direct map.
`kmap` adds `0xFFFF000000000000` to a physical address and `kernel_to_phys`
subtracts it.

## Virtual memory

ChaOS uses AArch64 stage-1 translation with 4 KiB granules and 48-bit virtual
addresses. `mmu_init` builds an identity map for early execution and a matching
higher-half kernel map, then configures:

- `MAIR_EL1` attribute 0 as device memory and attribute 1 as normal write-back
  memory;
- `TCR_EL1` for 48-bit TTBR0/TTBR1 spaces, 4 KiB granules, and inner-shareable
  write-back/write-allocate table walks;
- `TTBR0_EL1` and `TTBR1_EL1` with the two roots; and
- `SCTLR_EL1` to enable the MMU and data/instruction caches.

Device mappings are execute-never; normal mappings are inner-shareable.

@image html identity_higher_half_mapping.svg "Identity and higher-half mappings"

This diagram is QEMU-oriented. Its identity-versus-higher-half relationship is
still correct, but physical load addresses and the division between RAM and
device memory differ on Raspberry Pi. The code uses each platform's
`mmu_map.h` rather than one universal physical map.

Boot mappings use L2 blocks. The common mapper can create L3 mappings and
allocate intermediate tables on demand; its current callers use this facility
for user pages.

@image html pgtable.svg "Four-level page-table walk to an L3 page"

The four-level walk and 4 KiB L3 mapping remain accurate. However, the current
kernel heap does not use separately allocated L3 heap mappings: it obtains slab
pages from the physical allocator and accesses them through the higher-half
direct map. Dynamic L3 mappings are currently used for user address spaces.

An entry covers 512 GiB at L0, 1 GiB at L1, 2 MiB at L2, and 4 KiB at L3.
Descriptors encode validity, table/page type, access, shareability, memory
attribute, user access, and privileged/unprivileged execute-never permissions.

Mapping or unmapping a page invalidates that virtual address. Changing
`TTBR0_EL1` invalidates all stage-1 translations because address-space
identifiers are not yet implemented.

<!-- DOC-GAP(detail): Describe the platform MMIO/RAM maps behind
     phys_is_device and why the two platforms classify different portions of
     their ranges as device memory. -->

<!-- DOC-GAP(rationale): Justify the MAIR/TCR cacheability and shareability
     choices and explain the barrier/TLBI ordering for map, unmap, MMU enable,
     and TTBR0 changes. -->

<!-- DOC-GAP(detail): Document walk_to_l3 failure/overwrite behavior and the
     absence of ASIDs, permission updates, table reclamation on unmap, and SMP
     TLB shootdown. -->

## Kernel heap

`kmalloc`, `kzalloc`, and `kfree` wrap a small slab allocator. It currently has
32-byte and 128-byte caches. A cache grows on demand by allocating a page whose
header records a validation magic value, owning cache, usage count, and free
object list. Objects start at the next 16-byte-aligned address.

Allocation chooses the smallest adequate cache. Requests larger than 128 bytes
fail. When the final object in a slab is freed, its page is returned to the page
allocator. Free-list changes are protected by masking interrupts.

<!-- DOC-GAP(diagram): Add a slab-page layout showing its header, alignment
     padding, objects, free list, and cache-to-slab links. -->

<!-- DOC-GAP(detail): Explain cache selection, invalid/double-free behavior,
     interrupt-safety boundaries, and why only 32- and 128-byte objects exist. -->

## User address spaces

Each user task owns an L0 table installed in `TTBR0_EL1`. Valid user addresses
range from `0x00001000` to `0x80000000`; programs begin at `0x00400000`, and the
stack occupies the page below `0x80000000`.

`map_user_page` uses the common L3 mapper. Code and stack pages are marked user-
accessible, while the stack is also non-executable.

`copy_user_pgd` recursively duplicates valid tables and mapped pages for
`fork`. On failure it tears down the partial table hierarchy. However,
`free_user_pgd` only returns page-table pages: it does not release physical
pages referenced by L3 leaf entries. Consequently, both normal address-space
destruction, failed partial copies, and failed image loads after mapping has
begun leak their mapped program/stack pages. Copy-on-write is not implemented.

`user_range_ok` rejects ranges outside userspace or arithmetic wraparound. The
copy helpers also walk every affected page and require a valid accessible user
mapping before transferring data:

- `copy_from_user` copies into a kernel buffer.
- `copy_to_user` copies into a user buffer.

<!-- DOC-GAP(diagram): Add an ownership/lifetime diagram for the user PGD,
     intermediate tables, program pages, and stack across create, fork, exec,
     and exit. -->

<!-- DOC-GAP(detail): Document PTE permission policy, recursive copy/free error
     cleanup (including the current L3 leaf-page leak), and the security
     assumptions behind checking mappings before memcpy. -->

<!-- DOC-GAP(detail): Document how Raspberry Pi physical memory should be
     discovered at runtime instead of relying on the current fixed PHYS_END. -->

## References

- [AArch64 translation-table descriptor formats](https://developer.arm.com/documentation/ddi0487/maa/-Part-D-The-AArch64-System-Level-Architecture/-Chapter-D8-The-AArch64-Virtual-Memory-System-Architecture/-D8-3-Translation-table-descriptor-formats/-D8-3-1-VMSAv8-64-descriptor-formats)
- [Device memory](https://developer.arm.com/documentation/107565/0101/Memory-system/Memory-types-and-attributes/Device-memory)
- [MAIR_EL1](https://developer.arm.com/documentation/ddi0601/2025-12/AArch64-Registers/MAIR-EL1--Memory-Attribute-Indirection-Register--EL1-)
- [TCR_EL1](https://developer.arm.com/documentation/ddi0601/2025-12/AArch64-Registers/TCR-EL1--Translation-Control-Register--EL1-)
- [SCTLR_EL1](https://developer.arm.com/documentation/ddi0601/2025-12/AArch64-Registers/SCTLR-EL1--System-Control-Register--EL1-)

[ChaOS documentation](@ref mainpage)

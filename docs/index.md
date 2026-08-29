# ChaOS {#mainpage}

AArch64 (ARMv8) OS kernel for QEMU's `virt` machine and Raspberry Pi.

<!-- DOC-GAP(diagram): Add a one-screen architecture overview showing the
     dependency direction among architecture/platform code, IRQs, memory,
     tasks/scheduler, syscalls, initramfs, and userspace. -->

<!-- DOC-GAP(detail): Add build/run instructions and a consolidated list of
     current limitations. -->

## Subsystems

- [Boot and architecture](@ref boot_architecture)
- [Interrupts and drivers](@ref interrupts_drivers)
- [Memory management](@ref memory_management)
- [Processes and scheduling](@ref processes_scheduling)
- [System calls](@ref system_calls)
- [Userspace](@ref userspace)

<!-- ## Source API reference

Doxygen generates the source-level reference under
[Data Structures](annotated.html), [Files](files.html), and
[Globals](globals.html). These indexes complement the subsystem pages above:
the Markdown explains architecture and rationale, while source annotations
define precise API contracts.

When adding source annotations:

1. Document a public API on its declaration in the public header. Avoid
   duplicating the same contract on its definition.
2. Include `@brief`, every `@param` direction, `@return`, error cases,
   preconditions, ownership/lifetime, and whether the operation blocks or
   requires a particular IRQ state or execution context.
3. Document structures at their declaration and explain fields whose ownership,
   units, state constraints, or queue membership are not self-evident.
4. Use `@see` with a page reference to connect an API to the relevant subsystem,
   for example `@see @ref memory_management`.
5. Add `@file` documentation where the purpose or invariants of an implementation
   file are not captured by its public interfaces.
6. Keep the six Markdown pages flat: do not introduce `@subpage` trees for source
   directories. Use `@defgroup` only when a stable public API genuinely benefits
   from a Modules entry, rather than creating one group per folder.
7. After a symbol is documented, replace important plain-text mentions in the
   narrative with explicit Doxygen references where direct API navigation is
   useful.

During the annotation migration, `EXTRACT_ALL` remains enabled so undocumented
symbols do not disappear. Once the intended public API is annotated, set
`EXTRACT_ALL = NO`; the warning settings in `Doxyfile` will then enforce missing
symbols, parameters, return documentation, and enum values. -->

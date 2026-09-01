# Boot and Architecture {#boot_architecture}

ChaOS enters through the AArch64 `_start` symbol and establishes the execution
environment required by the C kernel. The architecture layer also owns
exception-vector entry, register frames, exception-level transitions, and the
architecture-specific MMU and TLB operations.

## Kernel image layout

Each platform linker script defines `KERNEL_PHYS_BASE` and includes the common
kernel layout:

- QEMU `virt`: `0x40000000`
- Raspberry Pi: `0x00080000`
- Higher-half offset: `0xFFFF000000000000`

The boot code and boot stack initially use physical addresses. The remaining
kernel sections are linked into the higher half and loaded at their corresponding
physical addresses. The initramfs and the beginning of allocatable memory are
page-aligned.

@image html kernel_linker.svg "Kernel image layout"

The diagram is a QEMU-oriented snapshot. The section ordering and higher-half
relationship remain valid, but exact section boundaries depend on the built
image and Raspberry Pi uses a different physical base.

<!-- DOC-GAP(detail): Replace the QEMU-oriented snapshot with a generated or
     symbolic layout that explains which addresses are link-time constants and
     which vary with PLATFORM and image size. -->

The exception-vector table is aligned to `0x800` by `vectors.S`, as required by
`VBAR_EL1`. Stack addresses remain 16-byte aligned in accordance with the
[AAPCS64 procedure-call standard](https://developer.arm.com/documentation/107656/0101/Getting-started-with-Armv8-M-based-systems/Procedure-Call-Standard-for-Arm-Architecture--AAPCS-).

## Early assembly entry

The `_start` routine:

1. Parks every core except core 0 and establishes the physical boot stack.
2. Detects whether execution began at EL2 or EL1.
3. When necessary, configures EL1 for AArch64, grants timer access, and returns
   from EL2 to EL1.
4. Masks interrupts, disables FP/NEON access, and zeros `.bss`.
5. Builds the identity and higher-half mappings and enables the MMU.
6. Branches to the higher-half entry point and switches stacks.
7. Clears `TTBR0_EL1`, invalidates the obsolete identity translations, and
   calls `kernel_entry`.

FP/NEON remains disabled because the kernel does not save or restore that state.
Only one core is active, so the early post-switch TLB invalidation is local.

<!-- DOC-GAP(rationale): Explain why ChaOS chooses a higher-half kernel, why the
     identity mapping is removed, and which EL/firmware state the boot path
     assumes on QEMU and Raspberry Pi. -->

<!-- DOC-GAP(diagram): Add an EL2/EL1 boot sequence showing the physical-to-
     virtual control transfer, stack changes, and TTBR ownership. -->

## Kernel initialization

`kernel_entry` initializes the UART, exception vectors, interrupt layer, UART
interrupts, memory allocators, scheduler, initramfs, and initial user process.
It then enables IRQs, yields into the scheduler, and acts as the idle task when
nothing else is runnable. The core also supplies panic handling, formatted
kernel output, alignment helpers, and freestanding string and memory routines.

<!-- DOC-GAP(detail): Document the temporary allocator/thread self-tests in
     kernel_entry and state whether they are intentional boot diagnostics or
     development code slated for removal. -->

<!-- DOC-GAP(detail): Document panic behavior, printk's supported formatting,
     and the contracts/limitations of the freestanding string helpers. -->

## Exceptions and trap frames {#exceptions_trap_frames}

`exception_init` installs the `vectors` table in `VBAR_EL1`. The table provides
entries for exceptions taken from EL1t, EL1h, and EL0. Unsupported synchronous
exceptions, FIQs, and SError exceptions call a panic handler.

IRQ and EL0 synchronous entries allocate an `IRQ_FRAME_SIZE` frame on the
current kernel stack. It stores the general-purpose registers, `ELR_EL1`,
`SPSR_EL1`, and `SP_EL0`, so the same layout serves as both interrupt frame and
syscall trap frame. The common return path calls the scheduler, which may select
a different task's frame before the assembly restores it and executes `eret`.

For EL0 synchronous exceptions, the exception class in `ESR_EL1` selects the
system-call, data-abort, or instruction-abort path. An abort from EL0 is reported
as a segmentation fault and exits the current task with status 1. An abort from
the kernel prints `ELR_EL1`, `ESR_EL1`, `FAR_EL1`, and `SPSR_EL1`, then panics.

`enter_user_mode` configures `SPSR_EL1` for EL0, loads the user program counter
and stack pointer, clears the general-purpose registers, and executes `eret`.

<!-- DOC-GAP(diagram): Add the exact irq_frame stack layout, including padding,
     offsets, and the relationship between the C structure and vectors.S. -->

<!-- DOC-GAP(detail): Add a vector-routing table for EL1t, EL1h, and EL0 and
     document the unsupported FIQ/SError policy and ESR decoding coverage. -->

<!-- DOC-GAP(rationale): Explain why IRQs and syscalls share one frame and why
     scheduling is performed from the common exception-return path. -->

## References

- [SPSR_EL1](https://developer.arm.com/documentation/ddi0601/2026-03/AArch64-Registers/SPSR-EL1--Saved-Program-Status-Register--EL1-)
- [ESR_EL1](https://developer.arm.com/documentation/ddi0601/2026-03/AArch64-Registers/ESR-EL1--Exception-Syndrome-Register--EL1-)

[ChaOS documentation](@ref mainpage)

# Interrupts and Drivers {#interrupts_drivers}

The generic interrupt layer owns a table of up to 128 IRQ handlers and hides the
selected hardware controller behind a platform interface.

## Generic interrupt handling

`irq_init` clears the handler table, initializes the controller, registers the
rescheduling interrupt, and initializes the timer. The UART registers its
interrupt separately.

On entry, the vector code saves a complete register frame. The generic handler
acknowledges the pending interrupt, ignores spurious interrupts, invokes the
registered handler, and reports completion to the controller. The return path
always calls the scheduler, allowing a timer tick, yield, or blocking operation
to select a different frame.

`irq_disable` and `irq_enable` manipulate the IRQ mask in `DAIF`. Critical
sections that preserve their caller's state use `irq_save` and `irq_restore`.

`yield` expires the current time slice, sets `need_schedule`, and raises a
software interrupt. QEMU uses GIC SGI 0; Raspberry Pi uses local mailbox 0.

<!-- DOC-GAP(diagram): Add an IRQ sequence from vector entry through controller
     acknowledge, handler dispatch, EOI, scheduling, and eret. -->

<!-- DOC-GAP(detail): Document IRQ number assignments, registration lifetime,
     spurious/unregistered IRQ behavior, nesting assumptions, and whether
     handlers may block. -->

## Interrupt controller

The generic layer calls `irq_controller_init`, `irq_controller_ack`,
`irq_controller_eoi`, and `irq_controller_send_sgi`.

QEMU uses GICv3. Initialization enables the EL1 system-register interface,
permits all priorities, enables Group 1 interrupts and the distributor, wakes
the core-0 redistributor, and configures the timer, reschedule SGI, and UART at
priority `0x80`. Acknowledgement reads `ICC_IAR1_EL1`; completion writes
`ICC_EOIR1_EL1`; rescheduling writes `ICC_SGI1R_EL1`.

Raspberry Pi uses the local peripheral block and legacy interrupt controller. It
enables the core physical timer, mailbox 0, and PL011 UART interrupt. Pending
hardware sources are mapped to generic IRQ numbers, and no explicit end-of-
interrupt write is required.

<!-- DOC-GAP(detail): Add the relevant MMIO bases/register roles for each
     platform and a table mapping hardware sources to generic IRQ IDs. -->

<!-- DOC-GAP(rationale): Explain why QEMU uses GICv3 while Raspberry Pi uses the
     legacy/local blocks, and why rescheduling targets core 0 only. -->

## Timer

The platform timer supplies the 10 Hz tick used for preemption, sleeping, and
deferred zombie cleanup. The driver reads `CNTFRQ_EL0`, calculates a nonzero
interval, programs the countdown, enables the timer, and registers its handler.
QEMU uses `CNTV_TVAL_EL0`/`CNTV_CTL_EL0`; Raspberry Pi uses
`CNTP_TVAL_EL0`/`CNTP_CTL_EL0`.

Each interrupt re-arms the timer, increments `system_tick`, and calls
`scheduler_tick`, which accounts for the current time slice, wakes sleepers, and
periodically checks for reapable zombies.

<!-- DOC-GAP(rationale): Explain the choice of 10 Hz and of the virtual timer on
     QEMU versus the non-secure physical timer on Raspberry Pi, including the
     expected effect on scheduling and sleep resolution. -->

## UART

Both platforms expose the same PL011 API. Raspberry Pi additionally configures
GPIO 14 and 15, disables their pulls, selects 115200 baud, and enables 8N1 mode
with FIFOs.

`uart_init` resets the device, initializes 256-byte receive and transmit rings,
and creates wait queues. `uart_irq_init` registers the handler, drains stale
input, clears pending interrupts, and enables receive and timeout interrupts.
The handler moves bytes between the hardware FIFO and software rings and wakes
blocked readers or writers after making progress.

`uart_getc` checks the software ring and hardware FIFO, then blocks the current
task when input is unavailable; early boot falls back to polling. `uart_putc` is
the polling console path. Buffered reads and writes use the rings and wait
queues, and output converts newlines to CRLF.

<!-- DOC-GAP(diagram): Add RX/TX ring-buffer producer/consumer flows showing IRQ
     handlers, task context, hardware FIFOs, and wait-queue wakeups. -->

<!-- DOC-GAP(detail): Document ring overflow behavior, partial writes, CRLF
     conversion, polling fallbacks, interrupt enable/disable transitions, and
     concurrency assumptions. -->

<!-- DOC-GAP(detail): Add public contracts for the IRQ, timer, and UART APIs,
     including callable context, blocking behavior, and error returns. -->

## References

- [ICC_SRE_EL1](https://developer.arm.com/documentation/ddi0601/2026-03/AArch64-Registers/ICC-SRE-EL1--Interrupt-Controller-System-Register-Enable-Register--EL1-)
- [ICC_IAR1_EL1](https://developer.arm.com/documentation/ddi0601/2026-03/AArch64-Registers/ICC-IAR1-EL1--Interrupt-Controller-Interrupt-Acknowledge-Register-1)
- [ICC_EOIR1_EL1](https://developer.arm.com/documentation/ddi0601/2026-03/AArch64-Registers/ICC-EOIR1-EL1--Interrupt-Controller-End-Of-Interrupt-Register-1)
- [CNTFRQ_EL0](https://developer.arm.com/documentation/ddi0601/2026-03/AArch64-Registers/CNTFRQ-EL0--Counter-timer-Frequency-Register)
- [CNTV_CTL_EL0](https://developer.arm.com/documentation/ddi0601/2026-03/AArch64-Registers/CNTV-CTL-EL0--Counter-timer-Virtual-Timer-Control-Register)

[ChaOS documentation](@ref mainpage)

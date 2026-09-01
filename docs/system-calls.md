# System-Call Interface {#system_calls}

The syscall interface is the boundary between EL0 programs and kernel services.
User pointers are validated through the [memory-management helpers](@ref memory_management).

## EL0 exception path

The userspace helper places arguments in `x0` through `x5`, the syscall number
in `x8`, and executes `svc #0`. The EL0 synchronous vector saves a complete trap
frame and dispatches SVC exception class `0x15` to `handle_el0_sync`.

The handler records the frame on the current task, extracts the syscall number
and arguments, and calls `syscall_dispatch`. The return value is written into the
saved `x0` slot before the common exception path restores the frame.

Unknown, out-of-range, and unregistered syscall numbers return `-1`.

<!-- DOC-GAP(diagram): Add the full EL0-to-EL1 syscall sequence, including frame
     creation, dispatch, a possible block/context switch, return-value update,
     and eret. -->

<!-- DOC-GAP(detail): Add an ABI table containing every number, register
     signature, return convention, blocking behavior, and error condition.
     Resolve/document SYS_GETPID, which is defined but has no registered handler
     or userspace wrapper. -->

## Syscalls

### `write`

Only file descriptor 1 is supported. The handler validates and copies the user
buffer into a 128-byte kernel staging buffer, then sends each chunk through the
UART. It returns the number of bytes accepted or `-1` on an initial error.

### `read`

Only file descriptor 0 is supported. Input is read from the UART and echoed.
Newline terminates the read and is copied to the destination; backspace/delete
removes the preceding buffered character and updates the terminal display.
Every destination byte is transferred with `copy_to_user`.

<!-- DOC-GAP(detail): Specify terminal line-discipline behavior, buffer limits,
     partial-read/write semantics, and how UART blocking affects syscall return. -->

### `execve`

The handler copies a NUL-terminated path of at most 128 bytes from the caller,
looks it up in the initramfs, and replaces the current user address space with
the selected flat binary. It resets the saved program counter and stack pointer
to `USER_VIRT_ENTRY` and `USER_STACK_TOP`. Leading slashes are accepted. The
current implementation ignores `argv`.

<!-- DOC-GAP(detail): Document flat-binary restrictions, replacement failure
     atomicity, ignored argv/environment, path normalization, and executable
     permission policy. -->

### `fork`

The handler allocates a child task and kernel stack, recursively duplicates the
parent's user page tables and mapped pages, copies the parent's trap frame, and
attaches the child to the process family. The child frame returns 0 from `fork`;
the parent receives the new PID. The child is then appended to the ready queue.

<!-- DOC-GAP(rationale): Explain the eager full-address-space copy and contrast
     it with the future copy-on-write/ASID design. -->

### `exit`

The handler records the supplied status and delegates to task exit. Children are
reparented to init, waiters are awakened, the process becomes a zombie, and the
scheduler is invoked.

### `wait` and `waitpid`

`wait` delegates to `waitpid(-1, ...)`. `waitpid` scans the caller's children for
a matching zombie, detaches and destroys it, optionally copies its exit status
to userspace, and returns its PID. A request for a positive PID that is not a
child returns `-1`. Otherwise, if no matching zombie exists, the parent blocks
on its child wait queue. In the current implementation this means
`waitpid(-1, ...)` also blocks when the caller has no children.

<!-- DOC-GAP(detail): Specify behavior for no children, invalid status pointers,
     multiple children exiting, orphan reparenting, and the wakeup/race
     invariants between task_exit and waitpid. -->

## Userspace wrappers

The small libc provides one wrapper per implemented call. All wrappers use the
same AArch64 register convention and `svc` helper described above.

<!-- DOC-GAP(detail): State how kernel and libc syscall-number definitions are
     kept synchronized and document clobbers plus the absence of errno. -->

## References

- [ESR_EL1](https://developer.arm.com/documentation/ddi0601/2026-03/AArch64-Registers/ESR-EL1--Exception-Syndrome-Register--EL1-)

[Boot and architecture](@ref boot_architecture) | [Userspace](@ref userspace)

[ChaOS documentation](@ref mainpage)

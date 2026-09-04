# System-Call Interface {#system_calls}

ChaOS exposes a small, non-POSIX syscall ABI to EL0 programs. User-memory
arguments are accessed through the [memory-management helpers](@ref memory_management).

## EL0 exception path

The userspace helper puts arguments in `x0`–`x5`, the syscall number in `x8`,
and executes `svc #0`. The EL0 vector saves a trap frame and sends SVC exception
class `0x15` to `handle_el0_sync`.

The handler dispatches the call and stores its result in saved `x0`. The common
return path may switch tasks before restoring a frame and executing `eret`.
Unknown or unregistered calls return `-1`; errors remain raw negative values,
with no userspace `errno` translation.

<!-- DOC-GAP(diagram): Add the full EL0-to-EL1 syscall sequence, including frame
     creation, dispatch, a possible block/context switch, return-value update,
     and eret. -->

## ABI summary

| Number | Name | Userspace signature | Status |
| ---: | --- | --- | --- |
| 0 | `write` | `long write(int fd, const void *buf, long len)` | Implemented |
| 1 | `exit` | `void exit(int status)` | Implemented |
| 2 | `getpid` | None | Reserved; not registered |
| 3 | `execve` | `long execve(const char *path, char *const argv[])` | Implemented; `argv` ignored |
| 4 | `fork` | `long fork(void)` | Implemented |
| 5 | `read` | `long read(int fd, void *buf, long len)` | Implemented |
| 6 | `wait` | `long wait(int *status)` | Implemented |
| 7 | `waitpid` | `long waitpid(int pid, int *status)` | Implemented |
| 8 | `open` | `long open(const char *path, int flags, int mode)` | Console only |
| 9 | `close` | `long close(int fd)` | Implemented |
| 10 | `dup` | `long dup(int old_fd)` | Implemented |
| 11 | `dup2` | `long dup2(int old_fd, int new_fd)` | Implemented |

## File descriptors and I/O

Each task has 32 descriptors. Duplicated and inherited descriptors share the
same reference-counted file, including its position and flags.

### `read`

`read(fd, buf, len)` accepts any readable descriptor. It rejects invalid
descriptors, negative lengths, and invalid user buffers; a zero-length request
returns zero.

One call performs one VFS read of at most 128 bytes, so larger requests return a
short result. Console reads block for input, echo characters, normalize CR/LF to
`\n`, and process backspace/delete. EOF returns zero and VFS errors pass through.

### `write`

`write(fd, buf, len)` accepts any writable descriptor and sends input through
the VFS in 128-byte chunks. It rejects invalid descriptors, negative lengths,
and invalid user buffers. Later errors return a partial byte count; initial
errors pass through. Console output may block, and LF is emitted as CRLF.

### `open`

`open(path, flags, mode)` accepts a path of at most 127 bytes. Only
`/dev/console` is recognized, and the new file uses the lowest free descriptor.
`flags` is stored but not interpreted; `mode` is ignored. Failures return `-1`.

### `close`

`close(fd)` removes the descriptor and drops its file reference. The last
reference releases backend state, the VFS path, and the file. Invalid or empty
descriptors return `-1`; success returns zero. Task exit also closes all files.

### `dup` and `dup2`

`dup(old_fd)` shares the file through the lowest free descriptor. `dup2` uses an
exact descriptor, replacing its previous file if necessary. If both descriptors
are equal and valid, `dup2` returns without changing anything. Both calls return
the new descriptor or `-1` on error.

## Process lifecycle

### `execve`

`execve(path, argv)` finds a nonempty flat binary in the boot initramfs. Paths
are limited to 127 bytes; one leading slash is accepted. The kernel builds a new
address space and resets the saved PC and stack. Open descriptors are preserved.

### `fork`

`fork()` eagerly copies the parent's address space and trap frame. File
descriptors are copied by reference, so file positions remain shared. The child
returns zero and the parent receives the child PID; setup failures return `-1`.

### `exit`

`exit(status)` records a 32-bit status, closes all descriptors, reparents
children to init, wakes waiters, marks the task as a zombie, and yields. It does
not return.

### `wait` and `waitpid`

`wait(status)` calls `waitpid(-1, status)`. A PID of `-1` selects any child; a
positive PID selects one child. A zombie is destroyed, its PID is returned, and
its status is copied when `status` is non-NULL. A positive non-child PID returns
`-1`; otherwise the parent blocks.

## References

- [SPSR_EL1](https://developer.arm.com/documentation/ddi0601/2026-03/AArch64-Registers/SPSR-EL1--Saved-Program-Status-Register--EL1-)
- [ESR_EL1](https://developer.arm.com/documentation/ddi0601/2026-03/AArch64-Registers/ESR-EL1--Exception-Syndrome-Register--EL1-)

[Boot and architecture](@ref boot_architecture) | [Filesystem](@ref filesystem) |
[Userspace](@ref userspace)

[ChaOS documentation](@ref mainpage)

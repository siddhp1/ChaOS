# Userspace {#userspace}

ChaOS builds `init`, `sh`, and `hello` as separate freestanding AArch64 flat
binaries and packages them in the kernel's initramfs.

## Initramfs

The build places the flat binaries under `bin/` in a CPIO `newc` archive.
`objcopy` converts that archive into an object linked into the kernel's page-
aligned `.initramfs` section.

`initramfs_init` walks the archive, validates the `070701` header magic, parses
the hexadecimal metadata, and records regular files in a fixed table of at most
32 entries. It stops at `TRAILER!!!` or malformed input. `initramfs_lookup`
performs a linear filename search.

`load_init` looks up `bin/init`, creates its user process, and records it in
`task_init`. `execve` uses the same archive lookup, accepting an optional leading
slash, and replaces the current process image while preserving its PID and
family relationships.

<!-- DOC-GAP(diagram): Add the build pipeline from ELF programs to flat binaries,
     CPIO newc archive, objcopy object, linker section, lookup, and process load. -->

<!-- DOC-GAP(detail): Document CPIO field/alignment rules, malformed-archive
     behavior, the 32-file limit, data ownership, and why a linear immutable
     initramfs is sufficient before a VFS exists. -->

## Program layout

The userspace linker script places each program at `0x00400000` and selects
`_start` as its entry point. Sections appear in this order:

1. `.text`, with `.text.boot` first
2. `.rodata`
3. `.data`
4. `.bss`, marked `NOLOAD`

The kernel maps enough pages at `USER_VIRT_ENTRY` to contain the resulting flat
file and separately maps a one-page user stack below `USER_STACK_TOP`. Because
`.bss` is `NOLOAD`, storage extending beyond the last file-backed page is not
represented in the binary and is not currently allocated by the loader. Any
portion within the final mapped page is zero only because the loader clears that
page before copying file data.

<!-- DOC-GAP(detail): Explain how NOLOAD .bss is represented in a flat binary
     and verify/document allocation and zeroing when .bss extends beyond the
     final file-backed page. -->

<!-- DOC-GAP(diagram): Add the EL0 virtual layout with guard regions, program
     pages, unmapped space, and the one-page stack. -->

## Startup

`user_boot.S` calls the program's `main`. If `main` returns, the startup stub
issues the exit syscall using the returned value left in `x0`, then waits forever
if the syscall unexpectedly returns.

## Minimal libc

The userspace library supplies syscall wrappers for `write`, `exit`, `execve`,
`fork`, `read`, `wait`, and `waitpid`, plus the string routines required by the
programs. Its syscall helper assigns arguments to `x0` through `x5`, assigns the
number to `x8`, executes `svc #0`, and returns the value from `x0`.

<!-- DOC-GAP(detail): Document the supported libc/string subset, ABI assumptions,
     syscall wrapper contracts, and unsupported facilities such as allocation,
     files, arguments, environment, and errno. -->

## Init and shell

`init` repeatedly forks a child, executes `bin/sh` in that child, and waits for
the shell to exit before starting another one.

The shell prints the ChaOS banner and reads commands from standard input:

- `hello` forks, executes `bin/hello`, and waits for it.
- `exit` prints a farewell and exits, after which init launches a new shell.
- any other non-empty command is reported as unknown.

<!-- DOC-GAP(rationale): Explain init's respawn policy and the shell's role as a
     minimal process/syscall integration test rather than a general shell. -->

[System calls](@ref system_calls) | [Processes and scheduling](@ref processes_scheduling)

[ChaOS documentation](@ref mainpage)

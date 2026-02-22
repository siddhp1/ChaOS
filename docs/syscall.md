# Syscalls

Registers:

- x8: syscall number
- x0: arg0
- x1: arg1
- x2: arg2
- x3: arg3
- x4: arg4
- x5: arg5

Return:

- x0: return value

Errors:

- Success: return >= 0
- Error: return negative errno

Invocation:

- svc #0

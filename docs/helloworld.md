### Requirements

- Cross compiler (set in Makefile)
- QEMU

### Commands

```
make clean

make

qemu-system-aarch64 -machine virt -cpu cortex-a53 -nographic -kernel kernel.elf
```

Use Ctrl+A then X to exit QEMU.

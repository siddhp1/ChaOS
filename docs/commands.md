# Commands

### Clean
```shell
make clean
```

### Build
aarch64-elf:
```shell
make
```
aarch64-linux:
```shell
make TOOLCHAIN=aarch64-linux-gnu
```

### Lint
```shell
clang-tidy kernel/**/*.c -- \
  -ffreestanding -nostdlib -nostartfiles \
  -target aarch64-none-elf \
  -Iinclude
```

### Run QEMU
```shell
qemu-system-aarch64 -machine virt -cpu cortex-a53 -nographic -kernel kernel.elf
```

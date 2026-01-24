# Commands

## Clean

```shell
make clean
```

## Build

aarch64-elf:

```shell
make
```

aarch64-linux:

```shell
make TOOLCHAIN=aarch64-linux-gnu
```

## Lint

>>>>>>> phase-two-a

```shell
clang-tidy kernel/**/*.c -- \
  -ffreestanding -nostdlib -nostartfiles \
  -target aarch64-none-elf \
  -Iinclude \
  -Iarch/arm64/include
```

## Run QEMU

>>>>>>> phase-two-a

```shell
qemu-system-aarch64 -machine virt -cpu cortex-a53 -m 512M -nographic -kernel kernel.elf
```

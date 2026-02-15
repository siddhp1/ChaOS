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

Debug builds:

```shell
make DEBUG=1
```

## Lint

```shell
clang-tidy $(find . -name '*.c' -o -name '*.h') -- \
  -ffreestanding -nostdlib \
  -target aarch64-none-elf \
  -Iinclude
```

## Run QEMU

```shell
qemu-system-aarch64 \
  -M virt,gic-version=3 \
  -cpu cortex-a53 \
  -m 512M \
  -kernel kernel.elf \
  -nographic
```

Add `-S -s` when running gdb

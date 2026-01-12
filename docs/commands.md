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

### Run QEMU
```shell
qemu-system-aarch64 -machine virt -cpu cortex-a53 -nographic -kernel kernel.elf
```

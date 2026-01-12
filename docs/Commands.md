### Lint

```shell
clang-tidy kernel/**/*.c -- \
  -ffreestanding -nostdlib -nostartfiles \
  -target aarch64-none-elf \
  -Iinclude
```

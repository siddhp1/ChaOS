# ChaOS

![ChaOS terminal output](./chaos.svg)

## About

Freestanding AArch64 (ARMv8) OS kernel for Raspberry Pi.

Includes UART console output, a preemptive scheduler, memory management, a userspace with syscalls, and initramfs loader.

> More information in the Wiki (coming soon!)

## Setup

### Prerequisites

- QEMU (AArch64): `qemu-system-aarch64`
- A cross compiler:
  - **Bare-metal (default):** `aarch64-elf-*`
  - **Linux cross:** `aarch64-linux-gnu-*`

### Build

**Bare-metal (default):**

```sh
make
```

**Linux cross toolchain:**

```sh
make TOOLCHAIN=aarch64-linux-gnu
```

**Debug build:**

```sh
make DEBUG=true
```

## Usage

### QEMU

```sh
qemu-system-aarch64 \
  -M virt,gic-version=3 \
  -cpu cortex-a53 \
  -m 512M \
  -kernel kernel.elf \
  -nographic
```

Add `-S -s` when running debug builds

## Next Steps

- Port to Raspberry Pi hardware
- Develop filesystem
- I/O and device drivers
- Graphics drivers
- Improved scheduler
- Power management

# ChaOS

![ChaOS terminal output](./chaos.svg)

## About

Freestanding AArch64 (ARMv8) OS kernel for Raspberry Pi.

Includes UART console output, a preemptive scheduler, memory management, a userspace with syscalls, and initramfs loader.

> More information in the [Wiki!](https://github.com/siddhp1/ChaOS/wiki)

## Setup

### Prerequisites

- QEMU (AArch64): `qemu-system-aarch64`
- A cross compiler:
  - Bare-metal (default): `aarch64-elf-*`
  - Linux cross: `aarch64-linux-gnu-*`

### Build

#### Command

```sh
make TOOLCHAIN=? PLATFORM=? DEBUG=?
```

#### Arguments

| Variable | Default | Supported values | Meaning |
| --- | --- | --- | --- |
| `TOOLCHAIN` | `aarch64-elf` | `aarch64-elf`, `aarch64-linux-gnu` | Selects the cross-compiler toolchain |
| `PLATFORM` | `qemu_virt` | `qemu_virt`, `rpi` | Builds for a specific platform |
| `DEBUG` | `true` | `true`, `false` | Adds debug compiler flags |

For QEMU, the recommended arguments are `PLATFORM=qemu_virt` and `DEBUG=true`.  
For Raspberry Pi, the recommended arguments are `PLATFORM=rpi` and `DEBUG=false`.

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

### Raspberry Pi

- Mount the Raspberry Pi boot partition (FAT) and copy the firmware files in `rpi/` and `kernel8.img`
- Connect a 3.3V USB-to-TTL serial adapter to the Pi UART (GPIO14 TXD0, GPIO15 RXD0, and GND)
- Use a serial console with baud rate of 115200 to view UART I/O

#### Minicom
```sh
minicom -D /dev/ttyUSB0 -b 115200
```

#### Screen
```sh
screen /dev/ttyUSB0 115200
```

## Next Steps

- Develop filesystem
- I/O and device drivers
- Graphics drivers
- Symmetric multiprocessing
- Power management

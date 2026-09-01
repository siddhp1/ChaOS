# ChaOS

![ChaOS terminal output](./chaos.svg)

## About

Freestanding AArch64 (ARMv8) OS kernel for Raspberry Pi (Cortex-A53) with PL011 UART for serial I/O.

Includes preemptive scheduler, memory management, a userspace with syscalls, and initramfs loader.

> More information in the [documentation!](https://siddhp1.github.io/ChaOS/)

## Setup

### Prerequisites

- [Docker](https://docs.docker.com/get-docker/) with Docker Compose v2
- [QEMU](https://www.qemu.org/download/) (`qemu-system-aarch64`) to run the kernel locally
- Optional: [Visual Studio Code](https://code.visualstudio.com/) with the
  [C/C++ extension](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cpptools)
  for the included tasks

Build the development container:

```sh
docker compose build
```

### Build

```sh
# QEMU
docker compose run --rm dev make PLATFORM=qemu_virt

# Raspberry Pi
docker compose run --rm dev make kernel8.img PLATFORM=rpi
```

The Make variables are:

| Variable | Default | Supported values | Meaning |
| --- | --- | --- | --- |
| `PLATFORM` | `qemu_virt` | `qemu_virt`, `rpi` | Builds for a specific platform |
| `DEBUG` | `true` | `true`, `false` | Adds debug compiler flags |

Clean build output with:

```sh
docker compose run --rm dev make clean
```

### VS Code tasks

- Build Debug
- Build Target
- Build Docs
- Clean
- Clean Docs
- Debug Kernel (QEMU)

### Documentation

Build the documentation inside the container:

```sh
docker compose run --rm dev make docs
```

Output is written to `docs/build`.

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

Use the **Debug Kernel (QEMU)** VS Code configuration to debug the kernel.

### Raspberry Pi

- Build `kernel8.img` with the Raspberry Pi command above or the **Build
  Target** task
- Mount the Raspberry Pi boot partition (FAT) and copy the firmware files in
  `rpi/` and the generated `kernel8.img`
- Connect a 3.3V USB-to-TTL serial adapter to the Pi UART (GPIO14 TXD0, GPIO15
  RXD0, and GND)
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

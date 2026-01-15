# Build
make clean
make
aarch64-elf-objcopy kernel.elf -O binary kernel8.img

# Install to SD card
rm -f /Volumes/RPI/bootcode.bin \
    /Volumes/RPI/config.txt \
    /Volumes/RPI/fixup.dat \
    /Volumes/RPI/kernel8.img \
    /Volumes/RPI/start.elf

cp bootcode.bin config.txt fixup.dat kernel8.img start.elf /Volumes/RPI/

# Eject SD card
diskutil eject /dev/disk4

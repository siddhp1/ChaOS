# Initramfs

## Image Format (repeating entries)

[filename\0] [uint64_t size] [data bytes]

## mkinitramfs.py

Helper function to pack flat binaries into a simple initramfs image.

### Usage

```bash
python3 mkinitramfs.py -o initramfs.img  name1=file1  name2=file2
```

This will create `initramfs.img` containing `file1` and `file2` with names `name1` and `name2`, respectively.

The names are the paths the kernel will use with initramfs_lookup().

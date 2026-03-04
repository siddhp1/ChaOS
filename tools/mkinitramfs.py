#!/usr/bin/env python3

import argparse
import struct
import sys
import os

def main():
    parser = argparse.ArgumentParser(description="Build an initramfs image")
    parser.add_argument("-o", "--output", required=True, help="Output file")
    parser.add_argument("entries", nargs="+",
                        help="name=path pairs (e.g. bin/init=build/init.bin)")
    args = parser.parse_args()

    blob = bytearray()
    for entry in args.entries:
        if "=" not in entry:
            print(f"Error: entry must be name=path, got: {entry}",
                  file=sys.stderr)
            sys.exit(1)
        name, path = entry.split("=", 1)
        if not os.path.isfile(path):
            print(f"Error: file not found: {path}", file=sys.stderr)
            sys.exit(1)
        data = open(path, "rb").read()
        # null-terminated name
        blob += name.encode("utf-8") + b"\x00"
        # Little-endian uint64 size
        blob += struct.pack("<Q", len(data))
        blob += data
        print(name, len(data), "bytes")

    with open(args.output, "wb") as f:
        f.write(blob)

    print(f"Wrote {args.output} ({len(blob)} bytes total)")


if __name__ == "__main__":
    main()

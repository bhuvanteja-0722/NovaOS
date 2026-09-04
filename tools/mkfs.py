#!/usr/bin/env python3
"""Create a deterministic NovaOS M7 filesystem image."""
from pathlib import Path
import struct
import sys

SECTOR_SIZE = 512
TOTAL_SECTORS = 16384
MAGIC = 0x5346564E
VERSION = 1
METADATA_LBA = 1
METADATA_MAGIC = 0x3141544D


def checksum_words(words):
    checksum = 0
    for word in words:
        checksum ^= (word + 0x9E3779B9 + ((checksum << 6) & 0xFFFFFFFF) + (checksum >> 2)) & 0xFFFFFFFF
    return checksum & 0xFFFFFFFF


def main(output):
    image = bytearray(SECTOR_SIZE * TOTAL_SECTORS)
    words = [MAGIC, VERSION, SECTOR_SIZE, TOTAL_SECTORS, METADATA_LBA, 1]
    superblock = struct.pack('<7I', *words, checksum_words(words))
    metadata = struct.pack('<4I', METADATA_MAGIC, VERSION, 1, 1)
    image[0:len(superblock)] = superblock
    image[SECTOR_SIZE:SECTOR_SIZE + len(metadata)] = metadata
    Path(output).write_bytes(image)


if __name__ == '__main__':
    if len(sys.argv) != 2:
        raise SystemExit('usage: mkfs.py OUTPUT')
    main(sys.argv[1])

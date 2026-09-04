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
NODE_LBA = 2
DATA_LBA = 3
METADATA_MAGIC = 0x3141544D
NODE_MAGIC = 0x31444F4E
NODE_SIZE = 48
NODE_COUNT = 8
NAME_SIZE = 24


def checksum_words(words):
    checksum = 0
    for word in words:
        checksum ^= (word + 0x9E3779B9 + ((checksum << 6) & 0xFFFFFFFF) + (checksum >> 2)) & 0xFFFFFFFF
    return checksum & 0xFFFFFFFF


def node(node_id, parent, kind, data_lba, size, name):
    encoded = name.encode('ascii')
    if len(encoded) >= NAME_SIZE:
        raise ValueError('node name too long')
    return struct.pack('<6I24s', NODE_MAGIC, node_id, parent, kind, data_lba, size,
                       encoded + b'\0' * (NAME_SIZE - len(encoded)))


def main(output):
    image = bytearray(SECTOR_SIZE * TOTAL_SECTORS)
    super_words = [MAGIC, VERSION, SECTOR_SIZE, TOTAL_SECTORS, METADATA_LBA, 1]
    image[0:28] = struct.pack('<7I', *super_words, checksum_words(super_words))

    metadata_words = [METADATA_MAGIC, VERSION, 1, 1, NODE_LBA, 1, DATA_LBA]
    image[SECTOR_SIZE:SECTOR_SIZE + 32] = struct.pack('<8I', *metadata_words, checksum_words(metadata_words))

    node_table = bytearray(SECTOR_SIZE)
    node_table[0:NODE_SIZE] = node(1, 1, 1, 0, 0, '/')
    node_table[NODE_SIZE:2 * NODE_SIZE] = node(2, 1, 1, 0, 0, 'etc')
    node_table[2 * NODE_SIZE:3 * NODE_SIZE] = node(3, 2, 2, DATA_LBA, 17, 'motd')
    image[NODE_LBA * SECTOR_SIZE:(NODE_LBA + 1) * SECTOR_SIZE] = node_table
    image[DATA_LBA * SECTOR_SIZE:DATA_LBA * SECTOR_SIZE + 17] = b'Welcome to NovaOS'
    Path(output).write_bytes(image)


if __name__ == '__main__':
    if len(sys.argv) != 2:
        raise SystemExit('usage: mkfs.py OUTPUT')
    main(sys.argv[1])

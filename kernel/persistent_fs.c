#include <stdint.h>

#define NOVA_PFS_MAGIC 0x5346564Eu
#define NOVA_PFS_VERSION 1u
#define NOVA_PFS_SUPERBLOCK_LBA 0u
#define NOVA_PFS_METADATA_LBA 1u
#define NOVA_PFS_METADATA_MAGIC 0x3141544Du
#define NOVA_PFS_SECTOR_SIZE 512u
#define NOVA_PFS_MAX_SECTORS 16384u

struct nova_pfs_superblock {
    uint32_t magic;
    uint32_t version;
    uint32_t sector_size;
    uint32_t total_sectors;
    uint32_t metadata_lba;
    uint32_t metadata_sectors;
    uint32_t checksum;
};

struct nova_pfs_metadata {
    uint32_t magic;
    uint32_t version;
    uint32_t generation;
    uint32_t root_node;
};

extern uint32_t storage_read_sector(uint32_t lba, void *buffer);

static uint32_t mounted;
static struct nova_pfs_superblock superblock;
static struct nova_pfs_metadata metadata;

static uint32_t checksum_words(const uint32_t *words, uint32_t count) {
    uint32_t checksum = 0;
    for (uint32_t index = 0; index < count; ++index) {
        checksum ^= words[index] + 0x9E3779B9u + (checksum << 6) + (checksum >> 2);
    }
    return checksum;
}

uint32_t persistent_fs_mount(void) {
    uint8_t sector[NOVA_PFS_SECTOR_SIZE];
    if (storage_read_sector(NOVA_PFS_SUPERBLOCK_LBA, sector) == 0) {
        return 0;
    }
    const struct nova_pfs_superblock *candidate = (const struct nova_pfs_superblock *)sector;
    if (candidate->magic != NOVA_PFS_MAGIC || candidate->version != NOVA_PFS_VERSION ||
        candidate->sector_size != NOVA_PFS_SECTOR_SIZE || candidate->total_sectors == 0 ||
        candidate->total_sectors > NOVA_PFS_MAX_SECTORS || candidate->metadata_lba != NOVA_PFS_METADATA_LBA ||
        candidate->metadata_sectors != 1 || candidate->checksum != checksum_words((const uint32_t *)candidate, 6)) {
        return 0;
    }
    superblock = *candidate;

    if (storage_read_sector(superblock.metadata_lba, sector) == 0) {
        return 0;
    }
    const struct nova_pfs_metadata *metadata_candidate = (const struct nova_pfs_metadata *)sector;
    if (metadata_candidate->magic != NOVA_PFS_METADATA_MAGIC ||
        metadata_candidate->version != NOVA_PFS_VERSION || metadata_candidate->root_node != 1) {
        return 0;
    }
    metadata = *metadata_candidate;
    mounted = 1;
    return 1;
}

uint32_t persistent_fs_is_mounted(void) {
    return mounted;
}

uint32_t persistent_fs_generation(void) {
    return metadata.generation;
}

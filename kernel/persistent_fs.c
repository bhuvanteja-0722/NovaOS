#include <stdint.h>

#define NOVA_PFS_MAGIC 0x5346564Eu
#define NOVA_PFS_VERSION 1u
#define NOVA_PFS_SUPERBLOCK_LBA 0u
#define NOVA_PFS_METADATA_LBA 1u
#define NOVA_PFS_NODE_LBA 2u
#define NOVA_PFS_DATA_LBA 3u
#define NOVA_PFS_METADATA_MAGIC 0x3141544Du
#define NOVA_PFS_NODE_MAGIC 0x31444F4Eu
#define NOVA_PFS_SECTOR_SIZE 512u
#define NOVA_PFS_MAX_SECTORS 16384u
#define NOVA_PFS_MAX_NODES 8u
#define NOVA_PFS_NAME_MAX 24u

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
    uint32_t node_lba;
    uint32_t node_sectors;
    uint32_t data_lba;
    uint32_t checksum;
};

struct nova_pfs_node {
    uint32_t magic;
    uint32_t id;
    uint32_t parent;
    uint32_t type;
    uint32_t data_lba;
    uint32_t size;
    char name[NOVA_PFS_NAME_MAX];
};

extern uint32_t storage_read_sector(uint32_t lba, void *buffer);

static uint32_t mounted;
static struct nova_pfs_superblock superblock;
static struct nova_pfs_metadata metadata;
static struct nova_pfs_node nodes[NOVA_PFS_MAX_NODES];

static uint32_t checksum_words(const uint32_t *words, uint32_t count) {
    uint32_t checksum = 0;
    for (uint32_t index = 0; index < count; ++index) {
        checksum ^= words[index] + 0x9E3779B9u + (checksum << 6) + (checksum >> 2);
    }
    return checksum;
}

static uint32_t name_matches(const char *left, const char *right) {
    for (uint32_t index = 0; index < NOVA_PFS_NAME_MAX; ++index) {
        if (left[index] != right[index]) {
            return 0;
        }
        if (left[index] == '\0') {
            return 1;
        }
    }
    return 0;
}

static uint32_t node_lookup(uint32_t parent, const char *name) {
    for (uint32_t index = 0; index < NOVA_PFS_MAX_NODES; ++index) {
        if (nodes[index].magic == NOVA_PFS_NODE_MAGIC && nodes[index].parent == parent &&
            name_matches(nodes[index].name, name)) {
            return nodes[index].id;
        }
    }
    return 0;
}

uint32_t persistent_fs_mount(void) {
    mounted = 0;
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
    if (metadata_candidate->magic != NOVA_PFS_METADATA_MAGIC || metadata_candidate->version != NOVA_PFS_VERSION ||
        metadata_candidate->generation == 0 || metadata_candidate->root_node != 1 || metadata_candidate->node_lba != NOVA_PFS_NODE_LBA ||
        metadata_candidate->node_sectors != 1 || metadata_candidate->data_lba != NOVA_PFS_DATA_LBA ||
        metadata_candidate->checksum != checksum_words((const uint32_t *)metadata_candidate, 7)) {
        return 0;
    }
    metadata = *metadata_candidate;

    if (storage_read_sector(metadata.node_lba, sector) == 0) {
        return 0;
    }
    for (uint32_t index = 0; index < NOVA_PFS_MAX_NODES; ++index) {
        const uint8_t *source = &sector[index * sizeof(struct nova_pfs_node)];
        uint8_t *destination = (uint8_t *)&nodes[index];
        for (uint32_t byte = 0; byte < sizeof(struct nova_pfs_node); ++byte) {
            destination[byte] = source[byte];
        }
    }
    if (nodes[0].magic != NOVA_PFS_NODE_MAGIC || nodes[0].id != 1 || nodes[0].parent != 1 || nodes[0].type != 1) {
        return 0;
    }
    for (uint32_t index = 0; index < NOVA_PFS_MAX_NODES; ++index) {
        if (nodes[index].magic != 0 && nodes[index].magic != NOVA_PFS_NODE_MAGIC) {
            return 0;
        }
    }
    mounted = 1;
    return 1;
}

uint32_t persistent_fs_is_mounted(void) { return mounted; }
uint32_t persistent_fs_generation(void) { return metadata.generation; }

uint32_t persistent_fs_lookup(const char *path) {
    if (!mounted || path == (const char *)0 || path[0] != '/') {
        return 0;
    }
    if (path[1] == '\0') {
        return 1;
    }
    if (path[1] == 'e' && path[2] == 't' && path[3] == 'c' && path[4] == '/' && path[5] != '\0') {
        if (path[5] == 'm' && path[6] == 'o' && path[7] == 't' && path[8] == 'd' && path[9] == '\0') {
            uint32_t etc = node_lookup(1, "etc");
            return etc == 0 ? 0 : node_lookup(etc, "motd");
        }
    }
    return 0;
}

int32_t persistent_fs_read_file(uint32_t node_id, void *buffer, uint32_t length, uint32_t offset) {
    if (!mounted || buffer == (void *)0 || node_id == 0 || node_id >= NOVA_PFS_MAX_NODES) {
        return -1;
    }
    const struct nova_pfs_node *node = &nodes[node_id - 1];
    if (node->magic != NOVA_PFS_NODE_MAGIC || node->type != 2 || offset > node->size) {
        return -1;
    }
    if (length > node->size - offset) {
        length = node->size - offset;
    }
    uint8_t sector[NOVA_PFS_SECTOR_SIZE];
    if (storage_read_sector(node->data_lba, sector) == 0) {
        return -1;
    }
    uint8_t *destination = (uint8_t *)buffer;
    for (uint32_t index = 0; index < length; ++index) {
        destination[index] = sector[offset + index];
    }
    return (int32_t)length;
}

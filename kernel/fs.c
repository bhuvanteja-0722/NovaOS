#include <stdint.h>

#define NOVA_FS_MAX_NODES 64
#define NOVA_FS_NAME_MAX 24
#define NOVA_FS_DATA_MAX 512
#define NOVA_FS_DIRECTORY 1u
#define NOVA_FS_FILE 2u

struct nova_fs_node {
    uint32_t used;
    uint32_t type;
    uint32_t parent;
    uint32_t size;
    char name[NOVA_FS_NAME_MAX];
    uint8_t data[NOVA_FS_DATA_MAX];
};

static struct nova_fs_node nodes[NOVA_FS_MAX_NODES];
static uint32_t node_count;

static uint32_t string_length(const char *text) {
    uint32_t length = 0;
    while (text != (const char *)0 && text[length] != '\0') {
        ++length;
    }
    return length;
}

static uint32_t child_lookup(uint32_t parent, const char *name, uint32_t name_length) {
    if (name_length == 0 || name_length >= NOVA_FS_NAME_MAX) {
        return 0;
    }
    for (uint32_t index = 1; index < node_count; ++index) {
        if (nodes[index].used && nodes[index].parent == parent &&
            string_length(nodes[index].name) == name_length) {
            uint32_t equal = 1;
            for (uint32_t name_index = 0; name_index < name_length; ++name_index) {
                if (nodes[index].name[name_index] != name[name_index]) {
                    equal = 0;
                    break;
                }
            }
            if (equal) {
                return index;
            }
        }
    }
    return 0;
}

static uint32_t path_lookup(const char *path) {
    if (path == (const char *)0 || path[0] != '/') {
        return 0;
    }
    if (path[1] == '\0') {
        return 1;
    }

    uint32_t current = 1;
    uint32_t index = 1;
    while (path[index] != '\0') {
        if (path[index] == '/') {
            return 0;
        }
        uint32_t start = index;
        while (path[index] != '\0' && path[index] != '/') {
            ++index;
        }
        uint32_t length = index - start;
        current = child_lookup(current, &path[start], length);
        if (current == 0) {
            return 0;
        }
        if (path[index] == '/') {
            ++index;
            if (path[index] == '\0') {
                return 0;
            }
        }
    }
    return current;
}

static uint32_t split_parent(const char *path, uint32_t *parent, const char **name, uint32_t *name_length) {
    uint32_t length = string_length(path);
    if (path == (const char *)0 || length < 2 || path[0] != '/' || path[length - 1] == '/') {
        return 0;
    }

    uint32_t slash = length - 1;
    while (slash > 0 && path[slash] != '/') {
        --slash;
    }
    if (slash == length - 1) {
        return 0;
    }
    if (slash == 0 || slash == 1) {
        *parent = 1;
    } else {
        char parent_path[96];
        if (slash >= sizeof(parent_path)) {
            return 0;
        }
        for (uint32_t index = 0; index < slash; ++index) {
            parent_path[index] = path[index];
        }
        parent_path[slash] = '\0';
        *parent = path_lookup(parent_path);
        if (*parent == 0) {
            return 0;
        }
    }
    *name = &path[slash + 1];
    *name_length = length - slash - 1;
    return *name_length > 0 && *name_length < NOVA_FS_NAME_MAX;
}

static uint32_t node_create(uint32_t parent, uint32_t type, const char *name, uint32_t name_length) {
    if (node_count >= NOVA_FS_MAX_NODES || parent == 0 || parent >= node_count ||
        !nodes[parent].used || nodes[parent].type != NOVA_FS_DIRECTORY || name_length == 0 ||
        name_length >= NOVA_FS_NAME_MAX || child_lookup(parent, name, name_length) != 0) {
        return 0;
    }

    uint32_t node = node_count++;
    nodes[node].used = 1;
    nodes[node].type = type;
    nodes[node].parent = parent;
    nodes[node].size = 0;
    for (uint32_t index = 0; index < NOVA_FS_DATA_MAX; ++index) {
        nodes[node].data[index] = 0;
    }
    for (uint32_t index = 0; index < name_length; ++index) {
        nodes[node].name[index] = name[index];
    }
    nodes[node].name[name_length] = '\0';
    return node;
}

void fs_init(void) {
    for (uint32_t index = 0; index < NOVA_FS_MAX_NODES; ++index) {
        nodes[index].used = 0;
    }
    node_count = 2;
    nodes[1].used = 1;
    nodes[1].type = NOVA_FS_DIRECTORY;
    nodes[1].parent = 1;
    nodes[1].name[0] = '/';
    nodes[1].name[1] = '\0';
}

uint32_t fs_mkdir(const char *path) {
    uint32_t parent;
    const char *name;
    uint32_t name_length;
    if (!split_parent(path, &parent, &name, &name_length)) {
        return 0;
    }
    return node_create(parent, NOVA_FS_DIRECTORY, name, name_length) != 0;
}

uint32_t fs_create(const char *path) {
    uint32_t parent;
    const char *name;
    uint32_t name_length;
    if (!split_parent(path, &parent, &name, &name_length)) {
        return 0;
    }
    return node_create(parent, NOVA_FS_FILE, name, name_length) != 0;
}

uint32_t fs_exists(const char *path) {
    return path_lookup(path) != 0;
}

int32_t fs_write(const char *path, const void *buffer, uint32_t length, uint32_t offset) {
    uint32_t node = path_lookup(path);
    if (node == 0 || nodes[node].type != NOVA_FS_FILE || buffer == (const void *)0 ||
        offset > NOVA_FS_DATA_MAX || length > NOVA_FS_DATA_MAX - offset) {
        return -1;
    }
    const uint8_t *source = (const uint8_t *)buffer;
    for (uint32_t index = 0; index < length; ++index) {
        nodes[node].data[offset + index] = source[index];
    }
    if (offset + length > nodes[node].size) {
        nodes[node].size = offset + length;
    }
    return (int32_t)length;
}

int32_t fs_read(const char *path, void *buffer, uint32_t length, uint32_t offset) {
    uint32_t node = path_lookup(path);
    if (node == 0 || nodes[node].type != NOVA_FS_FILE || buffer == (void *)0 || offset > nodes[node].size) {
        return -1;
    }
    uint32_t available = nodes[node].size - offset;
    if (length > available) {
        length = available;
    }
    uint8_t *destination = (uint8_t *)buffer;
    for (uint32_t index = 0; index < length; ++index) {
        destination[index] = nodes[node].data[offset + index];
    }
    return (int32_t)length;
}

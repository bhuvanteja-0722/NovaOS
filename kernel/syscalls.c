#include <stdint.h>

#define NOVA_MAX_FDS 8u
#define NOVA_FD_INVALID 0xFFFFFFFFu
#define NOVA_EBADF 9
#define NOVA_EINVAL 22
#define NOVA_ENOENT 2
#define NOVA_EOVERFLOW 75
#define NOVA_FD_READ 1u

struct nova_fd {
    uint32_t used;
    uint32_t node_id;
    uint32_t offset;
    uint32_t flags;
};

extern uint32_t persistent_fs_lookup(const char *path);
extern int32_t persistent_fs_read_file(uint32_t node_id, void *buffer, uint32_t length, uint32_t offset);

static struct nova_fd descriptors[NOVA_MAX_FDS];

static uint32_t buffer_valid(const void *buffer, uint32_t length) {
    uintptr_t start = (uintptr_t)buffer;
    uintptr_t end = start + (uintptr_t)length;
    return buffer != (const void *)0 && end >= start && end <= 0xC0000000u;
}

void syscalls_init(void) {
    for (uint32_t index = 0; index < NOVA_MAX_FDS; ++index) {
        descriptors[index].used = 0;
        descriptors[index].node_id = 0;
        descriptors[index].offset = 0;
        descriptors[index].flags = 0;
    }
}

int32_t nova_sys_open(const char *path) {
    uint32_t node_id = persistent_fs_lookup(path);
    if (node_id == 0) {
        return -NOVA_ENOENT;
    }
    for (uint32_t index = 0; index < NOVA_MAX_FDS; ++index) {
        if (!descriptors[index].used) {
            descriptors[index].used = 1;
            descriptors[index].node_id = node_id;
            descriptors[index].offset = 0;
            descriptors[index].flags = NOVA_FD_READ;
            return (int32_t)index;
        }
    }
    return -NOVA_EOVERFLOW;
}

int32_t nova_sys_read(uint32_t fd, void *buffer, uint32_t length) {
    if (fd >= NOVA_MAX_FDS || !descriptors[fd].used) {
        return -NOVA_EBADF;
    }
    if (!buffer_valid(buffer, length) || length > 512u) {
        return -NOVA_EINVAL;
    }
    int32_t result = persistent_fs_read_file(descriptors[fd].node_id, buffer, length, descriptors[fd].offset);
    if (result < 0) {
        return result;
    }
    descriptors[fd].offset += (uint32_t)result;
    return result;
}

int32_t nova_sys_close(uint32_t fd) {
    if (fd >= NOVA_MAX_FDS || !descriptors[fd].used) {
        return -NOVA_EBADF;
    }
    descriptors[fd].used = 0;
    descriptors[fd].node_id = 0;
    descriptors[fd].offset = 0;
    descriptors[fd].flags = 0;
    return 0;
}

uint32_t nova_sys_fd_is_open(uint32_t fd) {
    return fd < NOVA_MAX_FDS && descriptors[fd].used;
}

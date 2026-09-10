#include <stdint.h>
#include "abi.h"

#define NOVA_EBADF 9
#define NOVA_ENOENT 2
#define NOVA_EOVERFLOW 75

extern uint32_t process_current_pid(void);
extern struct nova_fd *process_fd_table(uint32_t pid);
extern uint32_t persistent_fs_lookup(const char *path);
extern int32_t persistent_fs_read_file(uint32_t node_id, void *buffer, uint32_t length, uint32_t offset);
extern uint32_t persistent_fs_file_size(uint32_t node_id);
extern int32_t persistent_fs_list_dir(const char *path, char *buffer, uint32_t length);

static uint32_t buffer_valid(const void *buffer, uint32_t length) {
    uintptr_t start = (uintptr_t)buffer;
    uintptr_t end = start + (uintptr_t)length;
    return buffer != (const void *)0 && end >= start && end <= 0xC0000000u;
}

void syscalls_init(void) {
}

int32_t nova_sys_open(const char *path) {
    uint32_t node_id = persistent_fs_lookup(path);
    struct nova_fd *descriptors = process_fd_table(process_current_pid());
    if (node_id == 0) {
        return -NOVA_ENOENT;
    }
    if (descriptors == (struct nova_fd *)0) {
        return -NOVA_EBADF;
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
    struct nova_fd *descriptors = process_fd_table(process_current_pid());
    if (descriptors == (struct nova_fd *)0 || fd >= NOVA_MAX_FDS || !descriptors[fd].used) {
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

int32_t nova_sys_seek(uint32_t fd, uint32_t offset, uint32_t whence) {
    struct nova_fd *descriptors = process_fd_table(process_current_pid());
    if (descriptors == (struct nova_fd *)0 || fd >= NOVA_MAX_FDS || !descriptors[fd].used || whence > 1u) {
        return -NOVA_EBADF;
    }
    uint32_t file_size = persistent_fs_file_size(descriptors[fd].node_id);
    uint32_t next_offset = whence == 0u ? offset : descriptors[fd].offset + offset;
    if (next_offset < descriptors[fd].offset && whence == 1u) {
        return -NOVA_EINVAL;
    }
    if (next_offset > file_size) {
        return -NOVA_EINVAL;
    }
    descriptors[fd].offset = next_offset;
    return (int32_t)next_offset;
}

int32_t nova_sys_listdir(const char *path, void *buffer, uint32_t length) {
    if (path == (const char *)0 || buffer == (void *)0 || length > 128u) {
        return -NOVA_EINVAL;
    }
    return persistent_fs_list_dir(path, (char *)buffer, length);
}

int32_t nova_sys_close(uint32_t fd) {
    struct nova_fd *descriptors = process_fd_table(process_current_pid());
    if (descriptors == (struct nova_fd *)0 || fd >= NOVA_MAX_FDS || !descriptors[fd].used) {
        return -NOVA_EBADF;
    }
    descriptors[fd].used = 0;
    descriptors[fd].node_id = 0;
    descriptors[fd].offset = 0;
    descriptors[fd].flags = 0;
    return 0;
}

uint32_t nova_sys_fd_is_open(uint32_t fd) {
    struct nova_fd *descriptors = process_fd_table(process_current_pid());
    return descriptors != (struct nova_fd *)0 && fd < NOVA_MAX_FDS && descriptors[fd].used;
}

int32_t nova_sys_fd_is_open_for(uint32_t pid, uint32_t fd) {
    struct nova_fd *descriptors = process_fd_table(pid);
    return descriptors != (struct nova_fd *)0 && fd < NOVA_MAX_FDS && descriptors[fd].used;
}

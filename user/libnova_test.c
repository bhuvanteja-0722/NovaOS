#include "libnova.h"

int32_t nova_wrapper_compile_test(const char *path, void *buffer) {
    int32_t fd = nova_open(path);
    (void)nova_seek((uint32_t)fd, 0, 0);
    (void)nova_read((uint32_t)fd, buffer, 1);
    (void)nova_listdir(path, buffer, 5);
    (void)nova_write(1, buffer, 1);
    (void)nova_close((uint32_t)fd);
    (void)nova_getpid();
    (void)nova_yield();
    return 0;
}

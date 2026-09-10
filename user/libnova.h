#ifndef NOVA_USER_LIBNOVA_H
#define NOVA_USER_LIBNOVA_H

#include <stdint.h>

#define NOVA_USER_SYSCALL_GETPID 1u
#define NOVA_USER_SYSCALL_YIELD 2u
#define NOVA_USER_SYSCALL_WRITE 3u
#define NOVA_USER_SYSCALL_OPEN 4u
#define NOVA_USER_SYSCALL_READ 5u
#define NOVA_USER_SYSCALL_CLOSE 6u
#define NOVA_USER_SYSCALL_SEEK 7u
#define NOVA_USER_SYSCALL_LISTDIR 8u
#define NOVA_USER_SYSCALL_EXIT 0u

static inline int32_t nova_syscall0(uint32_t number) {
    int32_t result;
    __asm__ volatile ("int $0x80" : "=a"(result) : "a"(number) : "memory", "cc");
    return result;
}

static inline int32_t nova_syscall3(uint32_t number, uint32_t argument0,
                                    uint32_t argument1, uint32_t argument2) {
    int32_t result;
    __asm__ volatile ("int $0x80" : "=a"(result) : "a"(number), "b"(argument0),
                      "c"(argument1), "d"(argument2) : "memory", "cc");
    return result;
}

static inline int32_t nova_getpid(void) { return nova_syscall0(NOVA_USER_SYSCALL_GETPID); }
static inline int32_t nova_yield(void) { return nova_syscall0(NOVA_USER_SYSCALL_YIELD); }
static inline int32_t nova_write(uint32_t fd, const void *buffer, uint32_t length) {
    return nova_syscall3(NOVA_USER_SYSCALL_WRITE, fd, (uint32_t)buffer, length);
}
static inline int32_t nova_open(const char *path) {
    return nova_syscall3(NOVA_USER_SYSCALL_OPEN, 0, (uint32_t)path, 0);
}
static inline int32_t nova_read(uint32_t fd, void *buffer, uint32_t length) {
    return nova_syscall3(NOVA_USER_SYSCALL_READ, fd, (uint32_t)buffer, length);
}
static inline int32_t nova_close(uint32_t fd) {
    return nova_syscall3(NOVA_USER_SYSCALL_CLOSE, fd, 0, 0);
}
static inline int32_t nova_seek(uint32_t fd, uint32_t offset, uint32_t whence) {
    return nova_syscall3(NOVA_USER_SYSCALL_SEEK, fd, offset, whence);
}
static inline int32_t nova_listdir(const char *path, void *buffer, uint32_t length) {
    return nova_syscall3(NOVA_USER_SYSCALL_LISTDIR, (uint32_t)path, (uint32_t)buffer, length);
}
static inline void nova_exit(void) { (void)nova_syscall0(NOVA_USER_SYSCALL_EXIT); }

#endif

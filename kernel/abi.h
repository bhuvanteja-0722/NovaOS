#ifndef NOVAOS_ABI_H
#define NOVAOS_ABI_H

#include <stdint.h>

#define NOVA_EXEC_MAGIC 0x4E4F5641u
#define NOVA_EXEC_VERSION 1u
#define NOVA_USER_BASE 0x00400000u
#define NOVA_USER_STACK_TOP 0x00801000u
#define NOVA_SYSCALL_GETPID 1u
#define NOVA_SYSCALL_YIELD 2u
#define NOVA_SYSCALL_WRITE 3u

struct nova_exec_header {
    uint32_t magic;
    uint32_t version;
    uint32_t entry_point;
    uint32_t image_size;
    uint32_t flags;
} __attribute__((packed));

struct nova_cpu_context {
    uint32_t edi;
    uint32_t esi;
    uint32_t ebp;
    uint32_t esp;
    uint32_t ebx;
    uint32_t edx;
    uint32_t ecx;
    uint32_t eax;
    uint32_t eip;
    uint32_t eflags;
    uint32_t user_esp;
    uint32_t user_ss;
};

#endif

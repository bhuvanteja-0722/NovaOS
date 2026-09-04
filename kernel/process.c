#include <stdint.h>
#include "abi.h"

#define NOVA_MAX_PROCESSES 16
#define NOVA_PROCESS_NAME_MAX 24

struct nova_process {
    uint32_t pid;
    uint32_t parent_pid;
    uint32_t state;
    uint32_t address_space;
    uint32_t entry_point;
    uint32_t user_stack;
    uint32_t timeslice_ticks;
    struct nova_cpu_context context;
    char name[NOVA_PROCESS_NAME_MAX];
};

static struct nova_process process_table[NOVA_MAX_PROCESSES];
static uint32_t process_total;
static uint32_t next_pid = 1;
static uint32_t current_pid;

static void copy_name(char *destination, const char *source) {
    uint32_t index = 0;
    while (index + 1 < NOVA_PROCESS_NAME_MAX && source[index] != '\0') {
        destination[index] = source[index];
        ++index;
    }
    destination[index] = '\0';
}

void process_init(void) {
    process_total = 0;
    next_pid = 1;
    current_pid = 0;
}

uint32_t process_create(const char *name, uint32_t parent_pid) {
    if (process_total >= NOVA_MAX_PROCESSES) {
        return 0;
    }

    struct nova_process *process = &process_table[process_total++];
    process->pid = next_pid++;
    process->parent_pid = parent_pid;
    process->state = 1;
    process->address_space = 0;
    process->entry_point = 0;
    process->user_stack = NOVA_USER_STACK_TOP;
    process->timeslice_ticks = 10;
    process->context.eip = 0;
    process->context.user_esp = NOVA_USER_STACK_TOP;
    process->context.user_ss = 0x23;
    copy_name(process->name, name);

    if (current_pid == 0) {
        current_pid = process->pid;
    }
    return process->pid;
}

uint32_t process_load_image(uint32_t pid, const struct nova_exec_header *header) {
    if (header == (const struct nova_exec_header *)0 || header->magic != NOVA_EXEC_MAGIC ||
        header->version != NOVA_EXEC_VERSION || header->entry_point < NOVA_USER_BASE ||
        header->image_size == 0 || header->image_size > 16u * 1024u * 1024u) {
        return 0;
    }

    for (uint32_t index = 0; index < process_total; ++index) {
        if (process_table[index].pid == pid) {
            process_table[index].entry_point = header->entry_point;
            process_table[index].context.eip = header->entry_point;
            process_table[index].state = 2;
            return 1;
        }
    }
    return 0;
}

uint32_t process_count(void) {
    return process_total;
}

uint32_t process_current_pid(void) {
    return current_pid;
}

uint32_t process_entry_point(uint32_t pid) {
    for (uint32_t index = 0; index < process_total; ++index) {
        if (process_table[index].pid == pid) {
            return process_table[index].entry_point;
        }
    }
    return 0;
}

uint32_t syscall_dispatch(uint32_t syscall_number, uint32_t argument0) {
    (void)argument0;
    switch (syscall_number) {
        case NOVA_SYSCALL_GETPID:
            return current_pid;
        case NOVA_SYSCALL_YIELD:
            return 0;
        case NOVA_SYSCALL_WRITE:
            return 0;
        default:
            return (uint32_t)-1;
    }
}

#include <stdint.h>

#define NOVA_MAX_PROCESSES 16
#define NOVA_PROCESS_NAME_MAX 24
#define NOVA_SYSCALL_GETPID 1
#define NOVA_SYSCALL_YIELD 2

struct nova_process {
    uint32_t pid;
    uint32_t parent_pid;
    uint32_t state;
    uint32_t address_space;
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
    copy_name(process->name, name);

    if (current_pid == 0) {
        current_pid = process->pid;
    }
    return process->pid;
}

uint32_t process_count(void) {
    return process_total;
}

uint32_t process_current_pid(void) {
    return current_pid;
}

uint32_t syscall_dispatch(uint32_t syscall_number, uint32_t argument0) {
    (void)argument0;
    switch (syscall_number) {
        case NOVA_SYSCALL_GETPID:
            return current_pid;
        case NOVA_SYSCALL_YIELD:
            return 0;
        default:
            return (uint32_t)-1;
    }
}

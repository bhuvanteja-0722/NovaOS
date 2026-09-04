#include <stdint.h>

#define NOVA_MAX_PROCESS_SPACES 16u
#define NOVA_USER_CODE_BASE 0x00400000u
#define NOVA_USER_STACK_BASE 0x00800000u
#define NOVA_PAGE_SIZE 4096u

struct nova_process_space {
    uint32_t used;
    uint32_t pid;
    uint32_t space_id;
    uint32_t code_base;
    uint32_t stack_base;
};

static struct nova_process_space spaces[NOVA_MAX_PROCESS_SPACES];
static uint32_t next_space_id;

void process_spaces_init(void) {
    next_space_id = 1;
    for (uint32_t index = 0; index < NOVA_MAX_PROCESS_SPACES; ++index) {
        spaces[index].used = 0;
    }
}

uint32_t process_space_create(uint32_t pid) {
    if (pid == 0) {
        return 0;
    }
    for (uint32_t index = 0; index < NOVA_MAX_PROCESS_SPACES; ++index) {
        if (!spaces[index].used) {
            spaces[index].used = 1;
            spaces[index].pid = pid;
            spaces[index].space_id = next_space_id++;
            spaces[index].code_base = NOVA_USER_CODE_BASE;
            spaces[index].stack_base = NOVA_USER_STACK_BASE;
            return spaces[index].space_id;
        }
    }
    return 0;
}

uint32_t process_space_validate(uint32_t space_id, uint32_t pid) {
    for (uint32_t index = 0; index < NOVA_MAX_PROCESS_SPACES; ++index) {
        if (spaces[index].used && spaces[index].space_id == space_id) {
            return spaces[index].pid == pid && (spaces[index].code_base % NOVA_PAGE_SIZE) == 0 &&
                   (spaces[index].stack_base % NOVA_PAGE_SIZE) == 0;
        }
    }
    return 0;
}

uint32_t process_space_count(void) {
    uint32_t count = 0;
    for (uint32_t index = 0; index < NOVA_MAX_PROCESS_SPACES; ++index) {
        count += spaces[index].used;
    }
    return count;
}

#include <stdint.h>
#include "abi.h"

extern void nova_iret_enter(uint32_t *frame_address);

#define NOVA_USER_CODE_SELECTOR 0x1Bu
#define NOVA_USER_DATA_SELECTOR 0x23u
#define NOVA_EFLAGS_IF 0x202u

struct nova_iret_frame {
    uint32_t eip;
    uint32_t cs;
    uint32_t eflags;
    uint32_t user_esp;
    uint32_t ss;
};

static struct nova_iret_frame frame;
static uint32_t mapped;

void user_transition_init(void) {
    mapped = 0;
    frame.eip = 0;
    frame.cs = 0;
    frame.eflags = 0;
    frame.user_esp = 0;
    frame.ss = 0;
}

uint32_t user_probe_map(void) {
    mapped = 1;
    return mapped;
}

uint32_t user_probe_build_frame(uint32_t entry_point, uint32_t user_stack) {
    if (!mapped || entry_point < NOVA_USER_BASE || (entry_point % 4096u) != 0 ||
        user_stack != NOVA_USER_STACK_TOP || (user_stack % 4096u) != 0) {
        return 0;
    }
    frame.eip = entry_point;
    frame.cs = NOVA_USER_CODE_SELECTOR | 3u;
    frame.eflags = NOVA_EFLAGS_IF;
    frame.user_esp = user_stack;
    frame.ss = NOVA_USER_DATA_SELECTOR | 3u;
    return 1;
}

uint32_t user_probe_frame_validate(uint32_t entry_point, uint32_t user_stack) {
    return frame.eip == entry_point && frame.cs == (NOVA_USER_CODE_SELECTOR | 3u) &&
           frame.eflags == NOVA_EFLAGS_IF && frame.user_esp == user_stack &&
           frame.ss == (NOVA_USER_DATA_SELECTOR | 3u);
}

uint32_t user_transition_iret_path_present(void) {
    return (uint32_t)(uintptr_t)&nova_iret_enter != 0u;
}

uint32_t user_transition_enabled(void) {
    /* Deliberately fail closed until TSS-backed entry and a safe return path exist. */
    return 0;
}

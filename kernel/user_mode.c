#include <stdint.h>

#define NOVA_USER_CODE_SELECTOR 0x1Bu
#define NOVA_USER_DATA_SELECTOR 0x23u

extern uint32_t address_space_range_count(void);

static uint32_t ready;
static uint32_t paging_active;

void user_mode_init(void) {
    ready = 0;
    paging_active = 0;
}

uint32_t user_mode_prepare(void) {
    if (address_space_range_count() < 2) {
        return 0;
    }
    ready = 1;
    return 1;
}

uint32_t user_mode_is_ready(void) {
    return ready;
}

uint32_t user_mode_transition_enabled(void) {
    return ready != 0 && paging_active != 0;
}

uint32_t user_mode_code_selector(void) {
    return NOVA_USER_CODE_SELECTOR;
}

uint32_t user_mode_data_selector(void) {
    return NOVA_USER_DATA_SELECTOR;
}

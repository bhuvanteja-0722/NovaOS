#include <stdint.h>

#define NOVA_PAGE_SIZE 4096u
#define NOVA_USER_MIN 0x00100000u
#define NOVA_USER_MAX 0xC0000000u
#define NOVA_MAX_USER_RANGES 8u

struct nova_user_range {
    uint32_t start;
    uint32_t end;
    uint32_t writable;
};

static struct nova_user_range ranges[NOVA_MAX_USER_RANGES];
static uint32_t range_count;

void address_space_init(void) {
    range_count = 0;
}

uint32_t address_space_map_user(uint32_t start, uint32_t length, uint32_t writable) {
    uint32_t end = start + length;
    if (range_count >= NOVA_MAX_USER_RANGES || length == 0 || end < start ||
        start < NOVA_USER_MIN || end > NOVA_USER_MAX || (start % NOVA_PAGE_SIZE) != 0 ||
        (length % NOVA_PAGE_SIZE) != 0) {
        return 0;
    }
    ranges[range_count].start = start;
    ranges[range_count].end = end;
    ranges[range_count].writable = writable != 0;
    ++range_count;
    return 1;
}

uint32_t address_space_validate_user(uint32_t address, uint32_t length, uint32_t write) {
    uint32_t end = address + length;
    if (length == 0 || end < address || address < NOVA_USER_MIN || end > NOVA_USER_MAX) {
        return 0;
    }
    for (uint32_t index = 0; index < range_count; ++index) {
        if (address >= ranges[index].start && end <= ranges[index].end &&
            (write == 0 || ranges[index].writable != 0)) {
            return 1;
        }
    }
    return 0;
}

uint32_t address_space_range_count(void) {
    return range_count;
}

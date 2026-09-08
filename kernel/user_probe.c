#include <stdint.h>
#include "abi.h"

#define NOVA_PROBE_ENTRY 0x00400000u
#define NOVA_PROBE_STACK 0x00801000u
#define NOVA_PROBE_MAX_SIZE 4096u

static const uint8_t probe_image[] = {
    0xB8, 0x01, 0x00, 0x00, 0x00, 0xCD, 0x80,
    0xB8, 0x02, 0x00, 0x00, 0x00, 0xCD, 0x80,
    0x31, 0xC0, 0xCD, 0x80, 0xF4
};
uint32_t user_probe_entry(void) {
    return NOVA_PROBE_ENTRY;
}

uint32_t user_probe_stack(void) {
    return NOVA_PROBE_STACK;
}

uint32_t user_probe_size(void) {
    return (uint32_t)sizeof(probe_image);
}

uint32_t user_probe_copy_to_user_page(void) {
    volatile uint8_t *destination = (volatile uint8_t *)NOVA_PROBE_ENTRY;
    for (uint32_t index = 0; index < sizeof(probe_image); ++index) {
        destination[index] = probe_image[index];
    }
    for (uint32_t index = 0; index < sizeof(probe_image); ++index) {
        if (destination[index] != probe_image[index]) {
            return 0;
        }
    }
    return 1;
}

uint32_t user_probe_validate_user_page(const uint8_t *expected, uint32_t length) {
    if (expected == (const uint8_t *)0 || length == 0 || length > NOVA_PROBE_MAX_SIZE) {
        return 0;
    }
    volatile const uint8_t *source = (volatile const uint8_t *)NOVA_PROBE_ENTRY;
    for (uint32_t index = 0; index < length; ++index) {
        if (source[index] != expected[index]) {
            return 0;
        }
    }
    return 1;
}

uint32_t user_probe_validate(void) {
    return sizeof(probe_image) > 0 && sizeof(probe_image) <= NOVA_PROBE_MAX_SIZE &&
           probe_image[0] == 0xB8 && probe_image[1] == 0x01 && probe_image[5] == 0xCD &&
           probe_image[6] == 0x80 && probe_image[7] == 0xB8 && probe_image[8] == 0x02 &&
           probe_image[12] == 0xCD && probe_image[13] == 0x80 && probe_image[14] == 0x31 &&
           probe_image[15] == 0xC0 && probe_image[16] == 0xCD && probe_image[17] == 0x80 &&
           probe_image[18] == 0xF4 &&
           (NOVA_PROBE_ENTRY % 4096u) == 0 && (NOVA_PROBE_STACK % 4096u) == 0;
}

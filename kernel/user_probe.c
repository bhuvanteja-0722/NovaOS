#include <stdint.h>
#include "abi.h"

#define NOVA_PROBE_ENTRY 0x00400000u
#define NOVA_PROBE_STACK 0x00801000u
#define NOVA_PROBE_MAX_SIZE 4096u

static const uint8_t probe_image[] = {
    0xBB, 0x01, 0x00, 0x00, 0x00, 0xB9, 0x29, 0x00, 0x40, 0x00,
    0xBA, 0x12, 0x00, 0x00, 0x00, 0xB8, 0x03, 0x00, 0x00, 0x00, 0xCD, 0x80,
    0xB8, 0x01, 0x00, 0x00, 0x00, 0xCD, 0x80,
    0xB8, 0x02, 0x00, 0x00, 0x00, 0xCD, 0x80,
    0x31, 0xC0, 0xCD, 0x80, 0xF4,
    'N','O','V','A','O','S','_','U','S','E','R','_','W','R','I','T','E','\n'
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
           probe_image[0] == 0xBB && probe_image[5] == 0xB9 && probe_image[10] == 0xBA &&
           probe_image[15] == 0xB8 && probe_image[20] == 0xCD && probe_image[21] == 0x80 &&
           probe_image[22] == 0xB8 && probe_image[29] == 0xB8 && probe_image[36] == 0x31 &&
           probe_image[37] == 0xC0 && probe_image[38] == 0xCD && probe_image[39] == 0x80 &&
           probe_image[40] == 0xF4 && probe_image[41] == 'N' && probe_image[58] == '\n' &&
           (NOVA_PROBE_ENTRY % 4096u) == 0 && (NOVA_PROBE_STACK % 4096u) == 0;
}

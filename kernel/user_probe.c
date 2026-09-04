#include <stdint.h>
#include "abi.h"

#define NOVA_PROBE_ENTRY 0x00400000u
#define NOVA_PROBE_STACK 0xBFFFE000u
#define NOVA_PROBE_MAX_SIZE 4096u

static const uint8_t probe_image[] = {
    0xCD, 0x80, /* int 0x80: syscall entry probe */
    0xF4       /* hlt: probe termination instruction */
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

uint32_t user_probe_validate(void) {
    return sizeof(probe_image) > 0 && sizeof(probe_image) <= NOVA_PROBE_MAX_SIZE &&
           probe_image[0] == 0xCD && probe_image[1] == 0x80 && probe_image[2] == 0xF4 &&
           (NOVA_PROBE_ENTRY % 4096u) == 0 && (NOVA_PROBE_STACK % 4096u) == 0;
}

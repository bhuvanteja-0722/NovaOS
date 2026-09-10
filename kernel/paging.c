#include <stdint.h>

#define NOVA_PAGE_SIZE 4096u
#define NOVA_PAGE_COUNT 4096u
#define NOVA_USER_START 0x00400000u
#define NOVA_USER_END 0x00C00000u
#define NOVA_PAGE_PRESENT 0x001u
#define NOVA_PAGE_WRITABLE 0x002u
#define NOVA_PAGE_USER 0x004u

static uint32_t page_directory[1024] __attribute__((aligned(NOVA_PAGE_SIZE)));
static uint32_t page_tables[4][1024] __attribute__((aligned(NOVA_PAGE_SIZE)));
static uint32_t prepared;
static uint32_t enabled;

void paging_init(void) {
    prepared = 0;
    enabled = 0;
    for (uint32_t directory = 0; directory < 4; ++directory) {
        uint32_t directory_flags = NOVA_PAGE_PRESENT | NOVA_PAGE_WRITABLE;
        if (directory >= 1 && directory <= 2) {
            directory_flags |= NOVA_PAGE_USER;
        }
        page_directory[directory] = ((uint32_t)&page_tables[directory][0]) | directory_flags;
        for (uint32_t entry = 0; entry < 1024; ++entry) {
            uint32_t address = (directory * 1024u + entry) * NOVA_PAGE_SIZE;
            uint32_t flags = NOVA_PAGE_PRESENT | NOVA_PAGE_WRITABLE;
            if (address >= NOVA_USER_START && address < NOVA_USER_END) {
                flags |= NOVA_PAGE_USER;
            }
            page_tables[directory][entry] = address | flags;
        }
    }
    for (uint32_t index = 4; index < 1024; ++index) {
        page_directory[index] = 0;
    }
    prepared = 1;
}

uint32_t paging_validate_layout(void) {
    if (!prepared || page_directory[0] == 0 || page_directory[1] == 0 || page_directory[2] == 0 || page_directory[3] == 0) {
        return 0;
    }
    uint32_t user_entry = page_tables[1][0];
    uint32_t kernel_entry = page_tables[0][0];
    if ((user_entry & (NOVA_PAGE_PRESENT | NOVA_PAGE_USER)) != (NOVA_PAGE_PRESENT | NOVA_PAGE_USER) ||
        (kernel_entry & NOVA_PAGE_USER) != 0 || (page_tables[2][1023] & NOVA_PAGE_USER) == 0 ||
        (page_directory[1] & NOVA_PAGE_USER) == 0 || (page_directory[2] & NOVA_PAGE_USER) == 0) {
        return 0;
    }
    return 1;
}

uint32_t paging_enable(void) {
    uint32_t control_register0;
    if (!prepared || enabled) {
        return enabled;
    }
    __asm__ volatile ("mov %0, %%cr3" : : "r"((uint32_t)&page_directory[0]) : "memory");
    __asm__ volatile ("mov %%cr0, %0" : "=r"(control_register0));
    control_register0 |= 0x80000000u;
    __asm__ volatile ("mov %0, %%cr0" : : "r"(control_register0) : "memory");
    enabled = 1;
    return enabled;
}

uint32_t paging_is_prepared(void) { return prepared; }
uint32_t paging_is_enabled(void) { return enabled; }
uint32_t paging_directory_address(void) { return (uint32_t)&page_directory[0]; }

uint32_t paging_user_range_mapped(uint32_t address, uint32_t length, uint32_t writable) {
    uint32_t end = address + length;
    if (!enabled || address < NOVA_USER_START || end < address || end > NOVA_USER_END) {
        return 0;
    }
    if (length == 0) {
        return 1;
    }
    uint32_t first_page = address & ~(NOVA_PAGE_SIZE - 1u);
    uint32_t last_page = (end - 1u) & ~(NOVA_PAGE_SIZE - 1u);
    for (uint32_t page = first_page;; page += NOVA_PAGE_SIZE) {
        uint32_t directory_index = page >> 22;
        uint32_t table_index = (page >> 12) & 0x3FFu;
        uint32_t directory_entry = page_directory[directory_index];
        uint32_t table_entry = page_tables[directory_index][table_index];
        uint32_t required = NOVA_PAGE_PRESENT | NOVA_PAGE_USER;
        if ((directory_entry & required) != required || (table_entry & required) != required ||
            (writable != 0 && (table_entry & NOVA_PAGE_WRITABLE) == 0)) {
            return 0;
        }
        if (page == last_page) {
            break;
        }
    }
    return 1;
}

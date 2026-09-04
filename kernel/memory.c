#include <stdint.h>

#define PAGE_SIZE 4096u
#define MULTIBOOT_FLAG_MEMORY 0x01u

extern uint8_t kernel_end;

static uint32_t memory_upper_kib;
static uint32_t allocator_start;
static uint32_t allocator_next;
static uint32_t allocator_limit;

static uint32_t align_up(uint32_t value, uint32_t alignment) {
    return (value + alignment - 1u) & ~(alignment - 1u);
}

void memory_init(uint32_t multiboot_info_address) {
    const uint32_t *info = (const uint32_t *)multiboot_info_address;
    uint32_t flags = info[0];
    memory_upper_kib = 0;

    if ((flags & MULTIBOOT_FLAG_MEMORY) != 0) {
        memory_upper_kib = info[2];
    }

    allocator_start = align_up((uint32_t)&kernel_end, PAGE_SIZE);
    allocator_next = allocator_start;
    allocator_limit = 16u * 1024u * 1024u;

    if (memory_upper_kib != 0) {
        uint32_t reported_limit = (memory_upper_kib + 1024u) * 1024u;
        if (reported_limit < allocator_limit) {
            allocator_limit = reported_limit;
        }
    }

    if (allocator_limit < allocator_start) {
        allocator_limit = allocator_start;
    }
}

void *memory_alloc_page(void) {
    if (allocator_next + PAGE_SIZE > allocator_limit) {
        return (void *)0;
    }

    uint32_t page = allocator_next;
    allocator_next += PAGE_SIZE;
    return (void *)page;
}

uint32_t memory_upper_kib_get(void) {
    return memory_upper_kib;
}

uint32_t memory_allocator_used(void) {
    return allocator_next - allocator_start;
}

uint32_t memory_allocator_start(void) {
    return allocator_start;
}

#include <stdint.h>

#define COM1 0x3F8

static inline void outb(uint16_t port, uint8_t value) {
    __asm__ volatile ("outb %0, %1" : : "a"(value), "Nd"(port));
}

static inline uint8_t inb(uint16_t port) {
    uint8_t value;
    __asm__ volatile ("inb %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

static void serial_init(void) {
    outb(COM1 + 1, 0x00);
    outb(COM1 + 3, 0x80);
    outb(COM1 + 0, 0x03);
    outb(COM1 + 1, 0x00);
    outb(COM1 + 3, 0x03);
    outb(COM1 + 2, 0xC7);
    outb(COM1 + 4, 0x0B);
}

static void serial_write_char(char c) {
    while ((inb(COM1 + 5) & 0x20) == 0) {
    }
    outb(COM1, (uint8_t)c);
}

static void serial_write(const char *text) {
    while (*text != '\0') {
        if (*text == '\n') {
            serial_write_char('\r');
        }
        serial_write_char(*text++);
    }
}

void kmain(uint32_t multiboot_magic, uint32_t multiboot_info) {
    (void)multiboot_info;
    serial_init();
    serial_write("NovaOS kernel starting...\n");

    if (multiboot_magic != 0x2BADB002) {
        serial_write("ERROR: invalid Multiboot magic\n");
        for (;;) {
            __asm__ volatile ("cli; hlt");
        }
    }

    serial_write("[ OK ] Multiboot handoff verified\n");
    serial_write("[ OK ] Bootstrap stack initialized\n");
    serial_write("[ OK ] Serial diagnostics online\n");
    serial_write("NOVAOS_PHASE0_BOOT_OK\n");

    for (;;) {
        __asm__ volatile ("hlt");
    }
}

#include <stdint.h>

#define ATA_DATA 0x1F0
#define ATA_SECTOR_COUNT 0x1F2
#define ATA_LBA_LOW 0x1F3
#define ATA_LBA_MID 0x1F4
#define ATA_LBA_HIGH 0x1F5
#define ATA_DRIVE 0x1F6
#define ATA_STATUS 0x1F7
#define ATA_COMMAND 0x1F7
#define ATA_CONTROL 0x3F6
#define ATA_READ_SECTORS 0x20
#define ATA_WRITE_SECTORS 0x30
#define ATA_CACHE_FLUSH 0xE7
#define ATA_STATUS_ERR 0x01
#define ATA_STATUS_DRQ 0x08
#define ATA_STATUS_BSY 0x80

static inline void outb(uint16_t port, uint8_t value) {
    __asm__ volatile ("outb %0, %1" : : "a"(value), "Nd"(port));
}

static inline uint8_t inb(uint16_t port) {
    uint8_t value;
    __asm__ volatile ("inb %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

static inline uint16_t inw(uint16_t port) {
    uint16_t value;
    __asm__ volatile ("inw %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

static inline void outw(uint16_t port, uint16_t value) {
    __asm__ volatile ("outw %0, %1" : : "a"(value), "Nd"(port));
}

static uint32_t ata_wait(uint8_t require_data) {
    for (uint32_t timeout = 0; timeout < 1000000u; ++timeout) {
        uint8_t status = inb(ATA_STATUS);
        if ((status & ATA_STATUS_ERR) != 0 || (status & ATA_STATUS_BSY) == 0) {
            return (status & ATA_STATUS_ERR) == 0 && (!require_data || (status & ATA_STATUS_DRQ) != 0);
        }
    }
    return 0;
}

static uint32_t ata_prepare(uint32_t lba, uint8_t command) {
    outb(ATA_CONTROL, 0);
    outb(ATA_DRIVE, (uint8_t)(0xE0u | ((lba >> 24) & 0x0Fu)));
    outb(ATA_SECTOR_COUNT, 1);
    outb(ATA_LBA_LOW, (uint8_t)lba);
    outb(ATA_LBA_MID, (uint8_t)(lba >> 8));
    outb(ATA_LBA_HIGH, (uint8_t)(lba >> 16));
    outb(ATA_COMMAND, command);
    return ata_wait(1);
}

uint32_t storage_read_sector(uint32_t lba, void *buffer) {
    if (buffer == (void *)0 || !ata_prepare(lba, ATA_READ_SECTORS)) {
        return 0;
    }
    uint16_t *destination = (uint16_t *)buffer;
    for (uint32_t index = 0; index < 256; ++index) {
        destination[index] = inw(ATA_DATA);
    }
    return 1;
}

uint32_t storage_write_sector(uint32_t lba, const void *buffer) {
    if (buffer == (const void *)0 || !ata_prepare(lba, ATA_WRITE_SECTORS)) {
        return 0;
    }
    const uint16_t *source = (const uint16_t *)buffer;
    for (uint32_t index = 0; index < 256; ++index) {
        outw(ATA_DATA, source[index]);
    }
    outb(ATA_COMMAND, ATA_CACHE_FLUSH);
    return ata_wait(0);
}

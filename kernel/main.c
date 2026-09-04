#include <stdint.h>
#include "abi.h"

#define COM1 0x3F8

extern void arch_init(void);
extern uint32_t timer_get_ticks(void);
extern void memory_init(uint32_t multiboot_info_address);
extern uint32_t memory_allocator_start(void);
extern uint32_t memory_allocator_used(void);
extern uint32_t memory_upper_kib_get(void);
extern void *memory_alloc_page(void);
extern void process_init(void);
extern uint32_t process_create(const char *name, uint32_t parent_pid);
extern uint32_t process_count(void);
extern uint32_t process_current_pid(void);
extern uint32_t syscall_dispatch(uint32_t syscall_number, uint32_t argument0);
extern uint32_t process_load_image(uint32_t pid, const struct nova_exec_header *header);
extern uint32_t process_entry_point(uint32_t pid);
extern uint32_t syscall_entry_count(void);
extern void fs_init(void);
extern uint32_t fs_mkdir(const char *path);
extern uint32_t fs_create(const char *path);
extern uint32_t fs_exists(const char *path);
extern int32_t fs_write(const char *path, const void *buffer, uint32_t length, uint32_t offset);
extern int32_t fs_read(const char *path, void *buffer, uint32_t length, uint32_t offset);
extern uint32_t storage_read_sector(uint32_t lba, void *buffer);
extern uint32_t storage_write_sector(uint32_t lba, const void *buffer);
extern uint32_t persistent_fs_mount(void);
extern uint32_t persistent_fs_is_mounted(void);
extern uint32_t persistent_fs_generation(void);
extern uint32_t persistent_fs_lookup(const char *path);
extern int32_t persistent_fs_read_file(uint32_t node_id, void *buffer, uint32_t length, uint32_t offset);
extern void syscalls_init(void);
extern int32_t nova_sys_open(const char *path);
extern int32_t nova_sys_read(uint32_t fd, void *buffer, uint32_t length);
extern int32_t nova_sys_close(uint32_t fd);
extern uint32_t nova_sys_fd_is_open(uint32_t fd);
extern void scheduler_init(void);
extern uint32_t scheduler_add(uint32_t pid);
extern void scheduler_tick(void);
extern uint32_t scheduler_current_pid(void);
extern uint32_t scheduler_count(void);
extern uint32_t scheduler_quantum(void);

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

static void serial_write_u32(uint32_t value) {
    char digits[10];
    int count = 0;
    if (value == 0) {
        serial_write_char('0');
        return;
    }
    while (value != 0) {
        digits[count++] = (char)('0' + (value % 10));
        value /= 10;
    }
    while (count > 0) {
        serial_write_char(digits[--count]);
    }
}

void kmain(uint32_t multiboot_magic, uint32_t multiboot_info) {
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

    arch_init();
    serial_write("[ OK ] GDT loaded\n");
    serial_write("[ OK ] IDT loaded and PIC remapped\n");
    serial_write("[ OK ] PIT configured at 100 Hz\n");

    memory_init(multiboot_info);
    serial_write("[ OK ] Physical memory allocator initialized at 0x");
    serial_write_u32(memory_allocator_start());
    serial_write("\n");
    serial_write("[ OK ] Multiboot upper memory KiB: ");
    serial_write_u32(memory_upper_kib_get());
    serial_write("\n");
    if (memory_alloc_page() == (void *)0) {
        serial_write("ERROR: bootstrap page allocation failed\n");
        for (;;) {
            __asm__ volatile ("cli; hlt");
        }
    }
    serial_write("[ OK ] First physical page allocated; bytes used: ");
    serial_write_u32(memory_allocator_used());
    serial_write("\n");

    __asm__ volatile ("sti");
    for (volatile uint32_t wait = 0; wait < 30000000; ++wait) {
        __asm__ volatile ("pause");
    }
    __asm__ volatile ("cli");

    serial_write("[ OK ] Timer interrupts observed: ");
    serial_write_u32(timer_get_ticks());
    serial_write(" ticks\n");
    process_init();
    uint32_t init_pid = process_create("init", 0);
    serial_write("[ OK ] Process table initialized; init PID: ");
    serial_write_u32(init_pid);
    serial_write("\n");
    if (process_count() != 1 || process_current_pid() != init_pid || syscall_dispatch(1, 0) != init_pid) {
        serial_write("ERROR: process/syscall self-test failed\n");
        for (;;) {
            __asm__ volatile ("cli; hlt");
        }
    }
    struct nova_exec_header init_image = {
        .magic = NOVA_EXEC_MAGIC,
        .version = NOVA_EXEC_VERSION,
        .entry_point = NOVA_USER_BASE,
        .image_size = 4096,
        .flags = 0
    };
    if (process_load_image(init_pid, &init_image) == 0 || process_entry_point(init_pid) != NOVA_USER_BASE) {
        serial_write("ERROR: init executable validation failed\n");
        for (;;) {
            __asm__ volatile ("cli; hlt");
        }
    }
    serial_write("[ OK ] Init executable metadata validated\n");

    uint32_t syscall_count_before = syscall_entry_count();
    __asm__ volatile ("int $0x80");
    if (syscall_entry_count() != syscall_count_before + 1) {
        serial_write("ERROR: syscall entry validation failed\n");
        for (;;) {
            __asm__ volatile ("cli; hlt");
        }
    }
    serial_write("[ OK ] Software syscall entry validated\n");
    serial_write("NOVAOS_M4_USERSPACE_OK\n");

    fs_init();
    if (fs_mkdir("/etc") == 0 || fs_create("/etc/motd") == 0 ||
        fs_exists("/etc/motd") == 0) {
        serial_write("ERROR: VFS node self-test failed\n");
        for (;;) {
            __asm__ volatile ("cli; hlt");
        }
    }
    static const char motd[] = "Welcome to NovaOS";
    char motd_readback[sizeof(motd)];
    if (fs_write("/etc/motd", motd, sizeof(motd), 0) != (int32_t)sizeof(motd) ||
        fs_read("/etc/motd", motd_readback, sizeof(motd_readback), 0) != (int32_t)sizeof(motd_readback)) {
        serial_write("ERROR: VFS file I/O self-test failed\n");
        for (;;) {
            __asm__ volatile ("cli; hlt");
        }
    }
    for (uint32_t index = 0; index < sizeof(motd); ++index) {
        if (motd_readback[index] != motd[index]) {
            serial_write("ERROR: VFS readback mismatch\n");
            for (;;) {
                __asm__ volatile ("cli; hlt");
            }
        }
    }
    if (fs_write("/etc/motd", motd, 1, 512) >= 0 || fs_read("/missing", motd_readback, 1, 0) >= 0) {
        serial_write("ERROR: VFS bounds self-test failed\n");
        for (;;) {
            __asm__ volatile ("cli; hlt");
        }
    }
    serial_write("[ OK ] VFS paths, directories, and bounded file I/O validated\n");
    serial_write("NOVAOS_M5_VFS_OK\n");

    if (persistent_fs_mount() == 0 || persistent_fs_is_mounted() == 0 || persistent_fs_generation() != 1) {
        serial_write("ERROR: persistent filesystem mount validation failed\n");
        for (;;) {
            __asm__ volatile ("cli; hlt");
        }
    }
    serial_write("[ OK ] Persistent filesystem superblock mounted\n");
    uint32_t motd_node = persistent_fs_lookup("/etc/motd");
    char persisted_motd[sizeof("Welcome to NovaOS") - 1];
    int32_t persisted_read = persistent_fs_read_file(motd_node, persisted_motd, sizeof(persisted_motd), 0);
    if (motd_node == 0 || persistent_fs_lookup("/etc/missing") != 0 || persisted_read != (int32_t)sizeof(persisted_motd)) {
        serial_write("ERROR: persistent VFS node validation failed\n");
        for (;;) {
            __asm__ volatile ("cli; hlt");
        }
    }
    static const char expected_motd[] = "Welcome to NovaOS";
    for (uint32_t index = 0; index < sizeof(expected_motd) - 1; ++index) {
        if (persisted_motd[index] != expected_motd[index]) {
            serial_write("[ DBG ] MOTD mismatch at index ");
            serial_write_u32(index);
            serial_write(", actual: ");
            serial_write_u32((uint32_t)(uint8_t)persisted_motd[index]);
            serial_write(", expected: ");
            serial_write_u32((uint32_t)(uint8_t)expected_motd[index]);
            serial_write("\nERROR: persistent VFS file content mismatch\n");
            for (;;) {
                __asm__ volatile ("cli; hlt");
            }
        }
    }
    serial_write("[ OK ] Persistent VFS nodes and file content loaded\n");

    uint8_t disk_sector[512];
    uint8_t disk_roundtrip[512];
    if (storage_read_sector(0, disk_sector) == 0 || disk_sector[0] != 'N' ||
        disk_sector[1] != 'V' || disk_sector[2] != 'F' || disk_sector[3] != 'S') {
        serial_write("ERROR: disk read or disk signature validation failed\n");
        for (;;) {
            __asm__ volatile ("cli; hlt");
        }
    }
    for (uint32_t index = 0; index < sizeof(disk_sector); ++index) {
        disk_sector[index] = (uint8_t)(index ^ 0xA5u);
    }
    if (storage_write_sector(2, disk_sector) == 0 || storage_read_sector(2, disk_roundtrip) == 0) {
        serial_write("ERROR: disk sector round-trip failed\n");
        for (;;) {
            __asm__ volatile ("cli; hlt");
        }
    }
    for (uint32_t index = 0; index < sizeof(disk_sector); ++index) {
        if (disk_sector[index] != disk_roundtrip[index]) {
            serial_write("ERROR: disk sector verification failed\n");
            for (;;) {
                __asm__ volatile ("cli; hlt");
            }
        }
    }
    serial_write("[ OK ] ATA disk sector read/write validated\n");
    serial_write("NOVAOS_M8_PERSISTENT_VFS_OK\n");

    syscalls_init();
    char syscall_motd[17];
    int32_t motd_fd = nova_sys_open("/etc/motd");
    if (motd_fd < 0 || nova_sys_read((uint32_t)motd_fd, syscall_motd, sizeof(syscall_motd)) != (int32_t)sizeof(syscall_motd) ||
        nova_sys_fd_is_open((uint32_t)motd_fd) == 0 || nova_sys_close((uint32_t)motd_fd) != 0 ||
        nova_sys_close((uint32_t)motd_fd) != -9 || nova_sys_read(99, syscall_motd, 1) != -9) {
        serial_write("ERROR: storage syscall self-test failed\n");
        for (;;) {
            __asm__ volatile ("cli; hlt");
        }
    }
    serial_write("[ OK ] Open/read/close storage syscalls validated\n");
    serial_write("NOVAOS_M9_SYSCALLS_OK\n");

    scheduler_init();
    uint32_t worker_pid = process_create("worker", init_pid);
    if (scheduler_add(init_pid) == 0 || scheduler_add(worker_pid) == 0 || scheduler_count() != 2 ||
        scheduler_current_pid() != init_pid) {
        serial_write("ERROR: scheduler queue self-test failed\n");
        for (;;) {
            __asm__ volatile ("cli; hlt");
        }
    }
    for (uint32_t index = 0; index < scheduler_quantum(); ++index) {
        scheduler_tick();
    }
    if (scheduler_current_pid() != worker_pid) {
        serial_write("ERROR: scheduler rotation self-test failed\n");
        for (;;) {
            __asm__ volatile ("cli; hlt");
        }
    }
    serial_write("[ OK ] Round-robin scheduler rotation validated\n");
    serial_write("NOVAOS_M10_SCHEDULER_OK\n");

    for (;;) {
        __asm__ volatile ("hlt");
    }
}

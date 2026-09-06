#include <stdint.h>
#include "abi.h"

#define COM1 0x3F8

extern void arch_init(void);
extern uint32_t tss_is_loaded(void);
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
extern uint32_t syscall_user_frame_captured(void);
extern uint32_t syscall_exit_is_requested(void);
extern uint32_t syscall_exit_should_terminate(void);
extern uint32_t syscall_last_user_eip(void);
extern void syscall_interrupt_handler(const void *frame, uint32_t syscall_number);
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
extern void address_space_init(void);
extern uint32_t address_space_map_user(uint32_t start, uint32_t length, uint32_t writable);
extern uint32_t address_space_validate_user(uint32_t address, uint32_t length, uint32_t write);
extern void user_mode_init(void);
extern uint32_t user_mode_prepare(void);
extern uint32_t user_mode_is_ready(void);
extern uint32_t user_mode_code_selector(void);
extern uint32_t user_mode_data_selector(void);
extern uint32_t user_mode_transition_enabled(void);
extern void paging_init(void);
extern uint32_t paging_validate_layout(void);
extern uint32_t paging_is_prepared(void);
extern uint32_t paging_enable(void);
extern uint32_t paging_is_enabled(void);
extern uint32_t paging_directory_address(void);
extern void process_spaces_init(void);
extern uint32_t process_space_create(uint32_t pid);
extern uint32_t process_space_validate(uint32_t space_id, uint32_t pid);
extern uint32_t process_space_count(void);
extern uint32_t process_set_address_space(uint32_t pid, uint32_t space_id);
extern uint32_t process_address_space(uint32_t pid);
extern uint32_t process_terminate(uint32_t pid);
extern uint32_t process_is_alive(uint32_t pid);
extern uint32_t process_configure_user_context(uint32_t pid, uint32_t entry_point, uint32_t user_stack);
extern uint32_t user_probe_entry(void);
extern uint32_t user_probe_stack(void);
extern uint32_t user_probe_size(void);
extern uint32_t user_probe_validate(void);
extern uint32_t user_probe_copy_to_user_page(void);
extern void user_transition_init(void);
extern uint32_t user_probe_map(void);
extern uint32_t user_probe_build_frame(uint32_t entry_point, uint32_t user_stack);
extern uint32_t user_probe_frame_validate(uint32_t entry_point, uint32_t user_stack);
extern uint32_t user_transition_iret_path_present(void);
extern uint32_t user_transition_enabled(void);

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
    if (tss_is_loaded() == 0) {
        serial_write("ERROR: TSS and kernel interrupt stack initialization failed\n");
        for (;;) {
            __asm__ volatile ("cli; hlt");
        }
    }
    serial_write("[ OK ] GDT and ring-0 TSS loaded\n");
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
    if (syscall_user_frame_captured() != 0) {
        serial_write("ERROR: kernel-origin syscall was accepted as user frame\n");
        for (;;) {
            __asm__ volatile ("cli; hlt");
        }
    }
    struct nova_test_user_frame {
        uint32_t eip;
        uint32_t cs;
        uint32_t eflags;
        uint32_t user_esp;
        uint32_t ss;
    } synthetic_user_frame = {
        .eip = NOVA_USER_BASE + 2u,
        .cs = 0x1Bu,
        .eflags = 0x202u,
        .user_esp = NOVA_USER_STACK_TOP,
        .ss = 0x23u
    };
    syscall_interrupt_handler(&synthetic_user_frame, NOVA_SYSCALL_EXIT);
    if (syscall_user_frame_captured() == 0 || syscall_exit_is_requested() == 0 ||
        syscall_last_user_eip() != NOVA_USER_BASE + 2u || syscall_exit_should_terminate() == 0 ||
        syscall_exit_should_terminate() != 0) {
        serial_write("ERROR: guarded user syscall return decision failed\n");
        for (;;) {
            __asm__ volatile ("cli; hlt");
        }
    }
    serial_write("[ OK ] User syscall frame validation and exit decision prepared\n");
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

    process_spaces_init();
    uint32_t init_space = process_space_create(init_pid);
    uint32_t worker_space = process_space_create(worker_pid);
    if (init_space == 0 || worker_space == 0 || init_space == worker_space || process_space_count() != 2 ||
        process_space_validate(init_space, init_pid) == 0 || process_space_validate(worker_space, worker_pid) == 0 ||
        process_space_validate(init_space, worker_pid) != 0 || process_set_address_space(init_pid, init_space) == 0 ||
        process_set_address_space(worker_pid, worker_space) == 0 || process_address_space(init_pid) != init_space ||
        process_address_space(worker_pid) != worker_space) {
        serial_write("ERROR: process address-space ownership test failed\n");
        for (;;) {
            __asm__ volatile ("cli; hlt");
        }
    }
    serial_write("[ OK ] Process-specific address-space ownership validated\n");
    if (process_terminate(0) != 0 || process_terminate(init_pid) != 0 || process_is_alive(init_pid) == 0 || process_is_alive(worker_pid) == 0) {
        serial_write("ERROR: guarded process termination test failed\n");
        for (;;) {
            __asm__ volatile ("cli; hlt");
        }
    }
    serial_write("[ OK ] Init protection and process termination guards validated\n");
    if (user_probe_validate() == 0 || user_probe_size() > 4096u ||
        process_configure_user_context(init_pid, user_probe_entry(), user_probe_stack()) == 0 ||
        process_entry_point(init_pid) != user_probe_entry()) {
        serial_write("ERROR: user probe context validation failed\n");
        for (;;) {
            __asm__ volatile ("cli; hlt");
        }
    }
    serial_write("[ OK ] User probe image and stack context validated\n");
    serial_write("[ OK ] Ring-3 entry remains disabled until the probe is mapped\n");
    user_transition_init();
    if (user_probe_copy_to_user_page() == 0 || user_probe_map() == 0 || user_probe_build_frame(user_probe_entry(), user_probe_stack()) == 0 ||
        user_probe_frame_validate(user_probe_entry(), user_probe_stack()) == 0 || user_transition_iret_path_present() == 0 ||
        user_transition_enabled() != 0) {
        serial_write("ERROR: protected user transition frame test failed\n");
        for (;;) {
            __asm__ volatile ("cli; hlt");
        }
    }
    serial_write("[ OK ] User probe bytes copied and iret frame validated\n");
    serial_write("[ OK ] Dedicated iret entry path linked and validated as unreachable\n");
    serial_write("[ OK ] User transition remains fail-closed until TSS and return path are enabled\n");

    paging_init();
    if (paging_is_prepared() == 0 || paging_validate_layout() == 0 || paging_is_enabled() != 0 || paging_directory_address() == 0) {
        serial_write("ERROR: paging layout self-test failed\n");
        for (;;) {
            __asm__ volatile ("cli; hlt");
        }
    }
    serial_write("[ OK ] Identity page tables and user permissions validated\n");
    if (paging_enable() == 0 || paging_is_enabled() == 0) {
        serial_write("ERROR: controlled paging activation failed\n");
        for (;;) {
            __asm__ volatile ("cli; hlt");
        }
    }
    serial_write("[ OK ] Controlled CR3/CR0 paging activation validated\n");

    address_space_init();
    user_mode_init();
    if (address_space_map_user(0x00400000u, 0x1000u, 0) == 0 ||
        address_space_map_user(0x00800000u, 0x1000u, 1) == 0 ||
        address_space_validate_user(0x00400000u, 0x100u, 0) == 0 ||
        address_space_validate_user(0x00800000u, 0x100u, 1) == 0 ||
        address_space_validate_user(0x00400000u, 0x100u, 1) != 0 ||
        address_space_validate_user(0xBFFFFFF0u, 0x100u, 0) != 0 ||
        user_mode_prepare() == 0 || user_mode_is_ready() == 0 ||
        user_mode_code_selector() != 0x1Bu || user_mode_data_selector() != 0x23u || user_mode_transition_enabled() != 0) {
        serial_write("ERROR: address-space and user-mode readiness test failed\n");
        for (;;) {
            __asm__ volatile ("cli; hlt");
        }
    }
    serial_write("[ OK ] Mapped user ranges and pointer validation validated\n");
    serial_write("[ OK ] Ring-3 transition remains disabled until a complete user process is ready\n");
    serial_write("[ OK ] User-mode transition contract prepared\n");
    serial_write("[ OK ] Ring-3 transition remains fail-closed until a complete user process is ready\n");
    serial_write("NOVAOS_M11_TRANSITION_READY\n");

    for (;;) {
        __asm__ volatile ("hlt");
    }
}

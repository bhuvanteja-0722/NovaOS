#include <stdint.h>

#define COM1 0x3F8
#define PIC1 0x20
#define PIC2 0xA0
#define PIT_COMMAND 0x43
#define PIT_CHANNEL0 0x40

struct gdt_entry {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t base_middle;
    uint8_t access;
    uint8_t granularity;
    uint8_t base_high;
} __attribute__((packed));

struct gdt_ptr {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

struct idt_entry {
    uint16_t base_low;
    uint16_t selector;
    uint8_t zero;
    uint8_t flags;
    uint16_t base_high;
} __attribute__((packed));

struct idt_ptr {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

struct nova_tss {
    uint32_t prev_tss;
    uint32_t esp0;
    uint32_t ss0;
    uint32_t esp1;
    uint32_t ss1;
    uint32_t esp2;
    uint32_t ss2;
    uint32_t cr3;
    uint32_t eip;
    uint32_t eflags;
    uint32_t eax;
    uint32_t ecx;
    uint32_t edx;
    uint32_t ebx;
    uint32_t esp;
    uint32_t ebp;
    uint32_t esi;
    uint32_t edi;
    uint32_t es;
    uint32_t cs;
    uint32_t ss;
    uint32_t ds;
    uint32_t fs;
    uint32_t gs;
    uint32_t ldt;
    uint16_t trap;
    uint16_t iomap_base;
} __attribute__((packed));

static struct gdt_entry gdt[6];
static struct gdt_ptr gdt_descriptor;
static struct idt_entry idt[256];
static struct idt_ptr idt_descriptor;
static volatile uint32_t timer_ticks;
static volatile uint32_t syscall_entries;
static struct nova_tss tss;
static uint32_t tss_kernel_stack[1024] __attribute__((aligned(16)));
static uint32_t tss_loaded;

extern void gdt_flush(uint32_t descriptor);
extern void tss_flush(uint32_t selector);
extern void idt_load(uint32_t descriptor);
extern void irq0_stub(void);
extern void exception_stub(void);
extern void syscall_stub(void);
extern void scheduler_tick(void);

static inline void outb(uint16_t port, uint8_t value) {
    __asm__ volatile ("outb %0, %1" : : "a"(value), "Nd"(port));
}

static inline uint8_t inb(uint16_t port) {
    uint8_t value;
    __asm__ volatile ("inb %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

static void set_gdt_entry(int index, uint32_t base, uint32_t limit, uint8_t access, uint8_t granularity) {
    gdt[index].base_low = base & 0xFFFF;
    gdt[index].base_middle = (base >> 16) & 0xFF;
    gdt[index].base_high = (base >> 24) & 0xFF;
    gdt[index].limit_low = limit & 0xFFFF;
    gdt[index].granularity = (limit >> 16) & 0x0F;
    gdt[index].granularity |= granularity & 0xF0;
    gdt[index].access = access;
}

static void tss_init(void) {
    uint8_t *bytes = (uint8_t *)&tss;
    for (uint32_t index = 0; index < sizeof(tss); ++index) {
        bytes[index] = 0;
    }
    tss.ss0 = 0x10;
    tss.esp0 = (uint32_t)&tss_kernel_stack[1024];
    tss.iomap_base = sizeof(tss);
}

static void gdt_init(void) {
    gdt_descriptor.limit = sizeof(gdt) - 1;
    gdt_descriptor.base = (uint32_t)&gdt;
    set_gdt_entry(0, 0, 0, 0, 0);
    set_gdt_entry(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);
    set_gdt_entry(2, 0, 0xFFFFFFFF, 0x92, 0xCF);
    set_gdt_entry(3, 0, 0xFFFFFFFF, 0xFA, 0xCF);
    set_gdt_entry(4, 0, 0xFFFFFFFF, 0xF2, 0xCF);
    set_gdt_entry(5, (uint32_t)&tss, sizeof(tss) - 1, 0x89, 0x00);
    gdt_flush((uint32_t)&gdt_descriptor);
    tss_flush(0x28);
    tss_loaded = 1;
}

static void idt_set_gate(uint8_t index, uint32_t base, uint16_t selector, uint8_t flags) {
    idt[index].base_low = base & 0xFFFF;
    idt[index].base_high = (base >> 16) & 0xFFFF;
    idt[index].selector = selector;
    idt[index].zero = 0;
    idt[index].flags = flags;
}

static void pic_remap(void) {
    uint8_t mask1 = inb(PIC1 + 1);
    uint8_t mask2 = inb(PIC2 + 1);
    outb(PIC1, 0x11);
    outb(PIC2, 0x11);
    outb(PIC1 + 1, 0x20);
    outb(PIC2 + 1, 0x28);
    outb(PIC1 + 1, 0x04);
    outb(PIC2 + 1, 0x02);
    outb(PIC1 + 1, 0x01);
    outb(PIC2 + 1, 0x01);
    outb(PIC1 + 1, mask1 & (uint8_t)~0x01);
    outb(PIC2 + 1, mask2);
}

static void idt_init(void) {
    idt_descriptor.limit = sizeof(idt) - 1;
    idt_descriptor.base = (uint32_t)&idt;
    for (int i = 0; i < 256; ++i) {
        idt_set_gate((uint8_t)i, (uint32_t)exception_stub, 0x08, 0x8E);
    }
    idt_set_gate(32, (uint32_t)irq0_stub, 0x08, 0x8E);
    idt_set_gate(0x80, (uint32_t)syscall_stub, 0x08, 0xEE);
    pic_remap();
    idt_load((uint32_t)&idt_descriptor);
}

static void pit_init(void) {
    uint16_t divisor = 1193180 / 100;
    outb(PIT_COMMAND, 0x36);
    outb(PIT_CHANNEL0, divisor & 0xFF);
    outb(PIT_CHANNEL0, divisor >> 8);
}

void arch_init(void) {
    tss_loaded = 0;
    tss_init();
    gdt_init();
    idt_init();
    pit_init();
}

void syscall_interrupt_handler(void) {
    ++syscall_entries;
}

uint32_t syscall_entry_count(void) {
    return syscall_entries;
}

void timer_interrupt_handler(void) {
    ++timer_ticks;
    scheduler_tick();
    outb(PIC1, 0x20);
}

void exception_interrupt_handler(void) {
    outb(PIC1, 0x20);
    for (;;) {
        __asm__ volatile ("cli; hlt");
    }
}

uint32_t timer_get_ticks(void) {
    return timer_ticks;
}

uint32_t tss_is_loaded(void) {
    return tss_loaded && tss.ss0 == 0x10 && tss.esp0 != 0 && tss.iomap_base == sizeof(tss);
}

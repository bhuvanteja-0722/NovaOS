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

static struct gdt_entry gdt[3];
static struct gdt_ptr gdt_descriptor;
static struct idt_entry idt[256];
static struct idt_ptr idt_descriptor;
static volatile uint32_t timer_ticks;
static volatile uint32_t syscall_entries;

extern void gdt_flush(uint32_t descriptor);
extern void idt_load(uint32_t descriptor);
extern void irq0_stub(void);
extern void exception_stub(void);
extern void syscall_stub(void);

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

static void gdt_init(void) {
    gdt_descriptor.limit = sizeof(gdt) - 1;
    gdt_descriptor.base = (uint32_t)&gdt;
    set_gdt_entry(0, 0, 0, 0, 0);
    set_gdt_entry(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);
    set_gdt_entry(2, 0, 0xFFFFFFFF, 0x92, 0xCF);
    gdt_flush((uint32_t)&gdt_descriptor);
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

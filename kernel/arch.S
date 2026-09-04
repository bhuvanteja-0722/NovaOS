.section .text

.global gdt_flush
.type gdt_flush, @function
gdt_flush:
    mov 4(%esp), %eax
    lgdt (%eax)
    mov $0x10, %ax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs
    mov %ax, %ss
    ljmp $0x08, $.gdt_flush_done
.gdt_flush_done:
    ret

.global idt_load
.type idt_load, @function
idt_load:
    mov 4(%esp), %eax
    lidt (%eax)
    ret

.global irq0_stub
.type irq0_stub, @function
irq0_stub:
    pusha
    call timer_interrupt_handler
    popa
    iret

.global syscall_stub
.type syscall_stub, @function
syscall_stub:
    pusha
    call syscall_interrupt_handler
    popa
    iret

.global exception_stub
.type exception_stub, @function
exception_stub:
    pusha
    call exception_interrupt_handler
    popa
    iret

.section .note.GNU-stack,"",@progbits

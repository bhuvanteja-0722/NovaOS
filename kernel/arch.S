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

.global tss_flush
.type tss_flush, @function
tss_flush:
    mov 4(%esp), %eax
    ltr %ax
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

/*
 * Enter ring 3 from a caller that supplies a validated iret frame.
 * The frame layout is EIP, CS, EFLAGS, user ESP, SS.
 * This symbol remains unused until TSS-backed kernel-stack switching
 * and a tested return/termination path are installed.
 */
.global nova_iret_enter
.type nova_iret_enter, @function
nova_iret_enter:
    mov 4(%esp), %esp
    iret

.section .note.GNU-stack,"",@progbits

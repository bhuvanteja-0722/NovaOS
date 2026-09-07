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
    lea 32(%esp), %edx
    mov 28(%esp), %eax
    push %eax
    push %edx
    call syscall_interrupt_handler
    add $8, %esp
    call syscall_exit_should_terminate
    test %eax, %eax
    jz .syscall_return
    lea 32(%esp), %edx
    push %edx
    call syscall_termination_trampoline
.syscall_halt:
    jmp .syscall_halt
.syscall_return:
    popa
    iret

.global page_fault_stub
.type page_fault_stub, @function
page_fault_stub:
    pusha
    mov 32(%esp), %eax
    push %eax
    call page_fault_interrupt_handler
    add $4, %esp
    popa
    add $4, %esp
    iret

.macro NOVA_VECTOR_STUB name, vector
.global \name
.type \name, @function
\name:
    pusha
    push $\vector
    call general_protection_interrupt_handler
    add $4, %esp
    popa
    iret
.endm

NOVA_VECTOR_STUB divide_error_stub, 0
NOVA_VECTOR_STUB invalid_opcode_stub, 6
NOVA_VECTOR_STUB device_not_available_stub, 7
NOVA_VECTOR_STUB invalid_tss_stub, 10
NOVA_VECTOR_STUB segment_not_present_stub, 11
NOVA_VECTOR_STUB stack_fault_stub, 12
NOVA_VECTOR_STUB general_protection_stub, 13
NOVA_VECTOR_STUB exception_vector_1_stub, 1
NOVA_VECTOR_STUB exception_vector_2_stub, 2
NOVA_VECTOR_STUB exception_vector_3_stub, 3
NOVA_VECTOR_STUB exception_vector_4_stub, 4
NOVA_VECTOR_STUB exception_vector_5_stub, 5
NOVA_VECTOR_STUB exception_vector_8_stub, 8
NOVA_VECTOR_STUB exception_vector_9_stub, 9
NOVA_VECTOR_STUB exception_vector_15_stub, 15
NOVA_VECTOR_STUB exception_vector_16_stub, 16
NOVA_VECTOR_STUB exception_vector_17_stub, 17
NOVA_VECTOR_STUB exception_vector_18_stub, 18
NOVA_VECTOR_STUB exception_vector_19_stub, 19
NOVA_VECTOR_STUB exception_vector_20_stub, 20
NOVA_VECTOR_STUB exception_vector_21_stub, 21
NOVA_VECTOR_STUB exception_vector_22_stub, 22
NOVA_VECTOR_STUB exception_vector_23_stub, 23
NOVA_VECTOR_STUB exception_vector_24_stub, 24
NOVA_VECTOR_STUB exception_vector_25_stub, 25
NOVA_VECTOR_STUB exception_vector_26_stub, 26
NOVA_VECTOR_STUB exception_vector_27_stub, 27
NOVA_VECTOR_STUB exception_vector_28_stub, 28
NOVA_VECTOR_STUB exception_vector_29_stub, 29
NOVA_VECTOR_STUB exception_vector_30_stub, 30
NOVA_VECTOR_STUB exception_vector_31_stub, 31

.section .rodata
.global nova_exception_stub_table
nova_exception_stub_table:
    .long divide_error_stub
    .long exception_vector_1_stub
    .long exception_vector_2_stub
    .long exception_vector_3_stub
    .long exception_vector_4_stub
    .long exception_vector_5_stub
    .long invalid_opcode_stub
    .long device_not_available_stub
    .long exception_vector_8_stub
    .long exception_vector_9_stub
    .long invalid_tss_stub
    .long segment_not_present_stub
    .long stack_fault_stub
    .long general_protection_stub
    .long page_fault_stub
    .long exception_vector_15_stub
    .long exception_vector_16_stub
    .long exception_vector_17_stub
    .long exception_vector_18_stub
    .long exception_vector_19_stub
    .long exception_vector_20_stub
    .long exception_vector_21_stub
    .long exception_vector_22_stub
    .long exception_vector_23_stub
    .long exception_vector_24_stub
    .long exception_vector_25_stub
    .long exception_vector_26_stub
    .long exception_vector_27_stub
    .long exception_vector_28_stub
    .long exception_vector_29_stub
    .long exception_vector_30_stub
    .long exception_vector_31_stub

.section .text

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

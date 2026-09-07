bits 32

section .multiboot
align 8

multiboot_header:
    dd 0xE85250D6                 ; Multiboot2 magic
    dd 0                          ; arquitectura i386
    dd multiboot_header_end - multiboot_header
    dd -(0xE85250D6 + 0 + (multiboot_header_end - multiboot_header))

    ; End tag
    dw 0
    dw 0
    dd 8

multiboot_header_end:

section .bss
align 16

stack_bottom:
    resb 16384
stack_top:

section .data
align 4096

pml4_table:
    dq pdpt_table + 0x003

pdpt_table:
    dq pd_table + 0x003

pd_table:
    ; 512 entradas de 2 MiB = 1 GiB identity mapped
%assign i 0
%rep 512
    dq (i * 0x200000) | 0x083
%assign i i + 1
%endrep

section .text
global _start
extern kernel_main

_start:
    cli

    ; Cargar nuestra pila
    mov esp, stack_top

    ; Comprobar soporte de CPUID
    pushfd
    pop eax
    mov ecx, eax

    xor eax, 1 << 21
    push eax
    popfd

    pushfd
    pop eax
    xor eax, ecx
    jz no_cpuid

    ; Comprobar long mode mediante CPUID
    mov eax, 0x80000000
    cpuid

    cmp eax, 0x80000001
    jb no_long_mode

    mov eax, 0x80000001
    cpuid

    test edx, 1 << 29
    jz no_long_mode

    ; Activar PAE
    mov eax, cr4
    or eax, 1 << 5
    mov cr4, eax

    ; Cargar PML4
    mov eax, pml4_table
    mov cr3, eax

    ; Activar Long Mode mediante EFER
    mov ecx, 0xC0000080
    rdmsr
    or eax, 1 << 8
    wrmsr

    ; Activar paginación + protección
    mov eax, cr0
    or eax, (1 << 31) | (1 << 0)
    mov cr0, eax

    ; Segmentos de 64 bits
    lgdt [gdt64.pointer]

    jmp gdt64.code:long_mode_start

bits 64

long_mode_start:
    ; Segmentos
    mov ax, gdt64.data
    mov ds, ax
    mov es, ax
    mov ss, ax

    ; Pila de 64 bits
    mov rsp, stack_top

    ; Llamar al kernel C++
    call kernel_main

.hang:
    cli
    hlt
    jmp .hang

no_cpuid:
no_long_mode:
    cli
    hlt
    jmp $

section .rodata

gdt64:
    dq 0x0000000000000000       ; Null
    dq 0x00AF9A000000FFFF       ; Code 64-bit
    dq 0x00AF92000000FFFF       ; Data

.pointer:
    dw .pointer - gdt64 - 1
    dq gdt64

.code equ 0x08
.data equ 0x10
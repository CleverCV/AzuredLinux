bits 32

section .multiboot
align 8

multiboot_header:
    dd 0xE85250D6
    dd 0
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

align 4096

pdpt_table:
    dq pd_table + 0x003

align 4096

pd_table:
%assign i 0
%rep 512
    dq (i * 0x200000) | 0x083
%assign i i + 1
%endrep


; ============================================================
; Multiboot2 memory map
; ============================================================

align 4

mmap_addr:
    dd 0

mmap_length:
    dd 0


section .text

global _start
extern kernel_main


_start:
    cli

    mov word [0xB8000], 0x0F41

    ; --------------------------------------------------------
    ; Guardar información de Multiboot2
    ;
    ; EAX = Multiboot2 magic
    ; EBX = dirección de Multiboot information structure
    ; --------------------------------------------------------

    mov esi, ebx

    ; Saltar header:
    ; +0 = total size
    ; +4 = reserved
    ; +8 = primer tag
    mov edi, esi
    add edi, 8


.find_multiboot_tag:

    ; type
    mov eax, [edi]

    ; type 0 = end
    test eax, eax
    jz .no_mmap

    ; type 6 = memory map
    cmp eax, 6
    je .found_mmap

    ; siguiente tag
    mov eax, [edi + 4]

    ; tamaño alineado a 8 bytes
    add eax, 7
    and eax, 0xFFFFFFF8

    add edi, eax

    jmp .find_multiboot_tag


.found_mmap:

    ; --------------------------------------------------------
    ; Tag:
    ;
    ; +0  type
    ; +4  size
    ; +8  entry_size
    ; +12 entry_version
    ; +16 primer entry
    ; --------------------------------------------------------

    lea eax, [edi + 16]
    mov [mmap_addr], eax

    mov eax, [edi + 4]
    sub eax, 16
    mov [mmap_length], eax

    jmp .memory_map_done


.no_mmap:

    mov dword [mmap_addr], 0
    mov dword [mmap_length], 0


.memory_map_done:


    ; ========================================================
    ; Cargar nuestra pila
    ; ========================================================

    mov esp, stack_top


    ; ========================================================
    ; Comprobar soporte de CPUID
    ; ========================================================

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


    ; ========================================================
    ; Comprobar Long Mode
    ; ========================================================

    mov eax, 0x80000000
    cpuid

    cmp eax, 0x80000001
    jb no_long_mode

    mov eax, 0x80000001
    cpuid

    test edx, 1 << 29
    jz no_long_mode


    ; ========================================================
    ; Activar PAE
    ; ========================================================

    mov eax, cr4
    or eax, 1 << 5
    mov cr4, eax


    ; ========================================================
    ; Cargar PML4
    ; ========================================================

    mov eax, pml4_table
    mov cr3, eax


    ; ========================================================
    ; Activar Long Mode mediante EFER
    ; ========================================================

    mov ecx, 0xC0000080

    rdmsr

    or eax, 1 << 8

    wrmsr


    ; ========================================================
    ; Activar paginación + protección
    ; ========================================================

    mov eax, cr0

    or eax, (1 << 31) | (1 << 0)

    mov cr0, eax


    ; ========================================================
    ; GDT 64-bit
    ; ========================================================

    lgdt [gdt64.pointer]

    jmp gdt64.code:long_mode_start


; ============================================================
; 64-bit
; ============================================================

bits 64

long_mode_start:

    ; Segmentos
    mov ax, gdt64.data
    mov ds, ax
    mov es, ax
    mov ss, ax

    ; Pila
    mov rsp, stack_top

    ; --------------------------------------------------------
    ; Pasar mapa de memoria al kernel
    ;
    ; RDI = mmap_addr
    ; RSI = mmap_length
    ; --------------------------------------------------------

    mov edi, [mmap_addr]
    mov esi, [mmap_length]

    call kernel_main


.hang:
    cli
    hlt
    jmp .hang


; ============================================================
; CPUID no disponible
; ============================================================

no_cpuid:

    cli
    hlt
    jmp no_cpuid


; ============================================================
; Long Mode no disponible
; ============================================================

no_long_mode:

    cli
    hlt
    jmp no_long_mode


; ============================================================
; IDT
; ============================================================

global idt_load

idt_load:

    lidt [rdi]
    ret


; ============================================================
; GDT
; ============================================================

section .rodata

gdt64:

    dq 0x0000000000000000
    dq 0x00AF9A000000FFFF
    dq 0x00AF92000000FFFF

.pointer:

    dw .pointer - gdt64 - 1
    dq gdt64

.code equ 0x08
.data equ 0x10
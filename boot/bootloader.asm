bits 16
org 0x7C00

start:
    cli

    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00

    sti

    mov [boot_drive], dl

; ==========================================
; Obtener mapa de memoria BIOS (E820)
; ==========================================

    mov di, 0x5000
    xor ebx, ebx
    mov word [memory_entries], 0

e820_loop:

    mov eax, 0xE820
    mov edx, 0x534D4150        ; "SMAP"
    mov ecx, 24                ; tamaño de entrada
    int 0x15

    jc e820_done

    cmp eax, 0x534D4150
    jne e820_done

    ; BIOS debe devolver al menos 20 bytes
    cmp ecx, 20
    jb e820_done

    inc word [memory_entries]

    ; Avanzar a la siguiente entrada
    add di, 24

    ; ¿Hay más entradas?
    test ebx, ebx
    jnz e820_loop

e820_done:

; ==========================================
; Mostrar mensaje
; ==========================================

    mov si, msg
    call print_string

; ==========================================
; Leer kernel desde disco
; ==========================================

    mov ah, 0x02
    mov al, 32               ; 32 sectores
    mov ch, 0
    mov cl, 2               ; sector 2
    mov dh, 0
    mov dl, [boot_drive]

    mov bx, 0x8000

    int 0x13

    jc disk_error

; ==========================================
; Saltar al kernel
; ==========================================

    jmp 0x0000:0x8000


; ==========================================
; Error de disco
; ==========================================

disk_error:
    mov si, error_msg
    call print_string

.hang:
    cli
    hlt
    jmp .hang


; ==========================================
; Imprimir string
; ==========================================

print_string:
    lodsb

    test al, al
    jz .done

    mov ah, 0x0E
    mov bh, 0
    int 0x10

    jmp print_string

.done:
    ret


; ==========================================
; Variables
; ==========================================

boot_drive db 0

memory_entries dw 0

msg db "AzuredLinux Bootloader", 13, 10, 0
error_msg db "Disk read error!", 13, 10, 0


; ==========================================
; Boot signature
; ==========================================

times 510-($-$$) db 0
dw 0xAA55
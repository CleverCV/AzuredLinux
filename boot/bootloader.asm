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

    mov si, msg
    call print_string

    ; Leer el kernel desde el disco
    mov ah, 0x02          ; BIOS read sectors
    mov al, 32            ; leer 32 sectores
    mov ch, 0             ; cilindro 0
    mov cl, 2             ; empezar en sector 2
    mov dh, 0             ; cabeza 0
    mov dl, [boot_drive]

    mov bx, 0x8000        ; cargar kernel en 0000:8000

    int 0x13

    jc disk_error

    ; Saltar al kernel
    jmp 0x0000:0x8000

disk_error:
    mov si, error_msg
    call print_string

.hang:
    cli
    hlt
    jmp .hang

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

boot_drive db 0

msg db "AzuredLinux Bootloader", 13, 10, 0
error_msg db "Disk read error!", 13, 10, 0

times 510-($-$$) db 0
dw 0xAA55
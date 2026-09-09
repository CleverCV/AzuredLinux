#include "idt.hpp"
#include "pic.hpp"
#include "keyboard.hpp"
#include "shell.hpp"
#include "../memory/memory.hpp"

#include <stdint.h>

static volatile uint16_t* vga =
    reinterpret_cast<volatile uint16_t*>(0xB8000);

static int cursor = 0;

static void put_char(char c)
{
    vga[cursor++] = static_cast<uint16_t>(0x0F00 | c);
}

static void print(const char* str)
{
    while (*str)
    {
        put_char(*str++);
    }
}

static void print_hex(uint32_t value)
{
    const char* hex = "0123456789ABCDEF";

    print("0x");

    for (int i = 7; i >= 0; i--)
    {
        put_char(hex[(value >> (i * 4)) & 0xF]);
    }
}

extern "C" void kernel_main(
    uint32_t mmap_addr,
    uint32_t mmap_length
)
{
    print("AzuredLinux Memory Debug\r\n");

    print("mmap_addr:   ");
    print_hex(mmap_addr);
    print("\r\n");

    print("mmap_length: ");
    print_hex(mmap_length);
    print("\r\n");

    Memory::init(
        mmap_addr,
        mmap_length
    );

    print("total:       ");
    print_hex(
        static_cast<uint32_t>(
            Memory::get_total_memory()
        )
    );
    print("\r\n");

    idt_init();
    pic_remap();
    keyboard_init();

    shell_init();

    asm volatile ("sti");

    while (true)
    {
        asm volatile ("hlt");
    }
}
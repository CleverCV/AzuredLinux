#include "idt.hpp"
#include "pic.hpp"
#include "keyboard.hpp"
#include "shell.hpp"

extern "C" void kernel_main()
{
    idt_init();
    pic_remap();
    keyboard_init();
    shell_init();

    volatile unsigned short* vga =
        reinterpret_cast<volatile unsigned short*>(0xB8000);

    const char* message = "Welcome to AzuredLinux!";

    for (int i = 0; message[i] != '\0'; ++i)
    {
        vga[i] = static_cast<unsigned short>(0x0F00 | message[i]);
    }



    asm volatile ("sti");

    while (true)
    {
        asm volatile ("hlt");
    }
}
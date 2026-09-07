extern "C" void kernel_main()
{
    volatile unsigned short* vga =
        reinterpret_cast<volatile unsigned short*>(0xB8000);

    const char* message = "Welcome to AzuredLinux!";

    for (int i = 0; message[i] != '\0'; ++i)
    {
        vga[i] = static_cast<unsigned short>(0x0F00 | message[i]);
    }

    while (true)
    {
        asm volatile ("hlt");
    }
}
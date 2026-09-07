#include "keyboard.hpp"
#include <stdint.h>

static inline uint8_t inb(uint16_t port)
{
    uint8_t value;

    asm volatile (
        "inb %1, %0"
        : "=a"(value)
        : "Nd"(port)
    );

    return value;
}

void keyboard_init()
{
}

extern "C" void keyboard_handler()
{
    uint8_t scancode = inb(0x60);

    // Ignorar key release
    if (scancode & 0x80)
        return;
}

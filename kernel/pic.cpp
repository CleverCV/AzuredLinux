#include "pic.hpp"
#include "keyboard.hpp"

static inline void outb(uint16_t port, uint8_t value)
{
    asm volatile (
        "outb %0, %1"
        :
        : "a"(value), "Nd"(port)
    );
}

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

#define PIC1        0x20
#define PIC2        0xA0

#define PIC1_COMMAND PIC1
#define PIC1_DATA    (PIC1 + 1)

#define PIC2_COMMAND PIC2
#define PIC2_DATA    (PIC2 + 1)

#define ICW1_INIT    0x10
#define ICW1_ICW4    0x01

#define ICW4_8086    0x01

void pic_remap()
{
    uint8_t mask1 = inb(PIC1_DATA);
    uint8_t mask2 = inb(PIC2_DATA);

    outb(PIC1_COMMAND, ICW1_INIT | ICW1_ICW4);
    outb(PIC2_COMMAND, ICW1_INIT | ICW1_ICW4);

    outb(PIC1_DATA, 32);
    outb(PIC2_DATA, 40);

    outb(PIC1_DATA, 4);
    outb(PIC2_DATA, 2);

    outb(PIC1_DATA, ICW4_8086);
    outb(PIC2_DATA, ICW4_8086);

    // Bloquear todas las IRQ
outb(PIC1_DATA, 0xFF);
outb(PIC2_DATA, 0xFF);

// Activar solamente IRQ0 (timer) e IRQ1 (keyboard)
outb(PIC1_DATA, 0xFC);    
}

void pic_send_eoi(uint8_t irq)
{
    if (irq >= 8)
        outb(PIC2_COMMAND, 0x20);

    outb(PIC1_COMMAND, 0x20);
}
extern "C" void irq_handler(uint64_t irq)
{
    if (irq == 1)
        keyboard_handler();

    pic_send_eoi(static_cast<uint8_t>(irq));
}
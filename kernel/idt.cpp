#include "idt.hpp"

struct IDTEntry
{
    uint16_t offset_low;
    uint16_t selector;
    uint8_t  ist;
    uint8_t  type_attr;
    uint16_t offset_mid;
    uint32_t offset_high;
    uint32_t zero;
} __attribute__((packed));

struct IDTPointer
{
    uint16_t limit;
    uint64_t base;
} __attribute__((packed));

static IDTEntry idt[256];
static IDTPointer idt_pointer;

// Stubs de IRQ
extern "C" void irq_stub_0();
extern "C" void irq_stub_1();
extern "C" void irq_stub_2();
extern "C" void irq_stub_3();
extern "C" void irq_stub_4();
extern "C" void irq_stub_5();
extern "C" void irq_stub_6();
extern "C" void irq_stub_7();

void idt_set_gate(
    int vector,
    uint64_t handler,
    uint16_t selector,
    uint8_t type_attr)
{
    idt[vector].offset_low  = handler & 0xFFFF;
    idt[vector].selector    = selector;
    idt[vector].ist         = 0;
    idt[vector].type_attr   = type_attr;
    idt[vector].offset_mid  = (handler >> 16) & 0xFFFF;
    idt[vector].offset_high = (handler >> 32) & 0xFFFFFFFF;
    idt[vector].zero        = 0;
}

extern "C" void idt_load(uint64_t idt_address);

void idt_init()
{
    for (int i = 0; i < 256; i++)
    {
        idt[i] = {};
    }

    // IRQ 0-7 → vectores 32-39
    idt_set_gate(32, reinterpret_cast<uint64_t>(irq_stub_0));
    idt_set_gate(33, reinterpret_cast<uint64_t>(irq_stub_1));
    idt_set_gate(34, reinterpret_cast<uint64_t>(irq_stub_2));
    idt_set_gate(35, reinterpret_cast<uint64_t>(irq_stub_3));
    idt_set_gate(36, reinterpret_cast<uint64_t>(irq_stub_4));
    idt_set_gate(37, reinterpret_cast<uint64_t>(irq_stub_5));
    idt_set_gate(38, reinterpret_cast<uint64_t>(irq_stub_6));
    idt_set_gate(39, reinterpret_cast<uint64_t>(irq_stub_7));

    idt_pointer.limit = sizeof(idt) - 1;
    idt_pointer.base = reinterpret_cast<uint64_t>(&idt);

    idt_load(reinterpret_cast<uint64_t>(&idt_pointer));
}
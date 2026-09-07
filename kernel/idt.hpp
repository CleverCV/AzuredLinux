#pragma once

#include <stdint.h>

void idt_init();

void idt_set_gate(
    int vector,
    uint64_t handler,
    uint16_t selector = 0x08,
    uint8_t type_attr = 0x8E
);
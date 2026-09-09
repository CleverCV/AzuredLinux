#pragma once

#include <stdint.h>

namespace PMM {

    constexpr uint64_t PAGE_SIZE = 4096;

    void init(uint32_t mmap_addr, uint32_t mmap_length);

    void* alloc_page();
    void free_page(void* address);

    uint64_t get_total_memory();
    uint64_t get_used_memory();
    uint64_t get_free_memory();
}
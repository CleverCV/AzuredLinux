#include "memory.hpp"
#include "pmm.hpp"

namespace Memory
{
    void init(
        uint32_t mmap_addr,
        uint32_t mmap_length
    )
    {
        PMM::init(
            mmap_addr,
            mmap_length
        );
    }


    void* alloc_page()
    {
        return PMM::alloc_page();
    }


    void free_page(void* address)
    {
        PMM::free_page(address);
    }


    void* alloc(uint64_t size)
    {
        if (size == 0)
            return nullptr;


        uint64_t pages =
            (size + PMM::PAGE_SIZE - 1) /
            PMM::PAGE_SIZE;


        void* first = PMM::alloc_page();

        if (first == nullptr)
            return nullptr;


        for (uint64_t i = 1;
             i < pages;
             i++)
        {
            if (PMM::alloc_page() == nullptr)
            {
                return nullptr;
            }
        }


        return first;
    }


    void free(void* address)
    {
        PMM::free_page(address);
    }


    uint64_t get_total_memory()
    {
        return PMM::get_total_memory();
    }


    uint64_t get_used_memory()
    {
        return PMM::get_used_memory();
    }


    uint64_t get_free_memory()
    {
        return PMM::get_free_memory();
    }
}
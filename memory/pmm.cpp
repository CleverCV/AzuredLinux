#include "pmm.hpp"

namespace PMM
{

    constexpr uint32_t MEMORY_USABLE = 1;

    struct MemoryMapEntry
    {
        uint64_t base;
        uint64_t length;
        uint32_t type;
        uint32_t reserved;
    };

    static uint64_t total_memory = 0;
    static uint64_t used_memory = 0;

    static uint8_t* bitmap = nullptr;

    static uint64_t total_pages = 0;
    static uint64_t bitmap_size = 0;

    constexpr uint64_t BITMAP_ADDRESS = 0x200000;


    static inline void bitmap_set(uint64_t page)
    {
        bitmap[page / 8] |=
            (1 << (page % 8));
    }


    static inline void bitmap_clear(uint64_t page)
    {
        bitmap[page / 8] &=
            ~(1 << (page % 8));
    }


    static inline bool bitmap_test(uint64_t page)
    {
        return bitmap[page / 8] &
               (1 << (page % 8));
    }


    void init(
        uint32_t mmap_addr,
        uint32_t mmap_length
    )
    {
        if (mmap_addr == 0 ||
            mmap_length == 0)
        {
            total_memory = 0;
            used_memory = 0;
            total_pages = 0;
            bitmap = nullptr;

            return;
        }


        MemoryMapEntry* entry =
            reinterpret_cast<MemoryMapEntry*>(
                static_cast<uint64_t>(mmap_addr)
            );


        uint32_t offset = 0;


        // ----------------------------------------------------
        // Encontrar la dirección máxima de RAM
        // ----------------------------------------------------

        while (offset + sizeof(MemoryMapEntry) <= mmap_length)
        {
            if (entry->base + entry->length > total_memory)
            {
                total_memory =
                    entry->base + entry->length;
            }

            offset += sizeof(MemoryMapEntry);

            entry++;
        }


        total_pages =
            (total_memory + PAGE_SIZE - 1) /
            PAGE_SIZE;


        // ----------------------------------------------------
        // Bitmap
        // ----------------------------------------------------

        bitmap_size =
            (total_pages + 7) / 8;

        bitmap =
            reinterpret_cast<uint8_t*>(
                BITMAP_ADDRESS
            );


        // Inicialmente todo ocupado
        for (uint64_t i = 0;
             i < bitmap_size;
             i++)
        {
            bitmap[i] = 0xFF;
        }


        // ----------------------------------------------------
        // Marcar regiones disponibles
        // ----------------------------------------------------

        entry =
            reinterpret_cast<MemoryMapEntry*>(
                static_cast<uint64_t>(mmap_addr)
            );

        offset = 0;


        while (offset + sizeof(MemoryMapEntry) <= mmap_length)
        {
            if (entry->type == MEMORY_USABLE)
            {
                uint64_t start =
                    entry->base;

                uint64_t end =
                    entry->base + entry->length;


                uint64_t first_page =
                    (start + PAGE_SIZE - 1) /
                    PAGE_SIZE;


                uint64_t last_page =
                    end / PAGE_SIZE;


                for (uint64_t page = first_page;
                     page < last_page &&
                     page < total_pages;
                     page++)
                {
                    bitmap_clear(page);
                }
            }


            offset += sizeof(MemoryMapEntry);

            entry++;
        }


        // ----------------------------------------------------
        // Reservar primeras 2 MiB
        // ----------------------------------------------------

        uint64_t reserved_pages =
            0x200000 / PAGE_SIZE;


        for (uint64_t page = 0;
             page < reserved_pages &&
             page < total_pages;
             page++)
        {
            bitmap_set(page);
        }


        used_memory =
            reserved_pages * PAGE_SIZE;
    }


    void* alloc_page()
    {
        if (bitmap == nullptr)
            return nullptr;


        for (uint64_t page = 0;
             page < total_pages;
             page++)
        {
            if (!bitmap_test(page))
            {
                bitmap_set(page);

                used_memory += PAGE_SIZE;

                return reinterpret_cast<void*>(
                    page * PAGE_SIZE
                );
            }
        }


        return nullptr;
    }


    void free_page(void* address)
    {
        if (address == nullptr)
            return;


        uint64_t addr =
            reinterpret_cast<uint64_t>(address);


        if (addr % PAGE_SIZE != 0)
            return;


        if (addr < 0x200000)
            return;


        uint64_t page =
            addr / PAGE_SIZE;


        if (page >= total_pages)
            return;


        if (!bitmap_test(page))
            return;


        bitmap_clear(page);


        if (used_memory >= PAGE_SIZE)
            used_memory -= PAGE_SIZE;
    }


    uint64_t get_total_memory()
    {
        return total_memory;
    }


    uint64_t get_used_memory()
    {
        return used_memory;
    }


    uint64_t get_free_memory()
    {
        if (total_memory < used_memory)
            return 0;

        return total_memory - used_memory;
    }
}
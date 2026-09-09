#include "shell.hpp"
#include "../memory/memory.hpp"
#include <stdint.h>

static volatile uint16_t* vga =
    (volatile uint16_t*)0xB8000;

static int cursor_x = 0;
static int cursor_y = 1;

static char command_buffer[128];
static int command_length = 0;


// ============================================================
// VGA
// ============================================================

static void put_char(char c)
{
    if (c == '\n')
    {
        cursor_x = 0;
        cursor_y++;

        if (cursor_y >= 25)
            cursor_y = 0;

        return;
    }

    if (c == '\b')
    {
        if (cursor_x > 0)
        {
            cursor_x--;

            vga[cursor_y * 80 + cursor_x] =
                0x0F00 | ' ';
        }

        return;
    }

    vga[cursor_y * 80 + cursor_x] =
        0x0F00 | (uint8_t)c;

    cursor_x++;

    if (cursor_x >= 80)
    {
        cursor_x = 0;
        cursor_y++;

        if (cursor_y >= 25)
            cursor_y = 0;
    }
}


// ============================================================
// PRINT
// ============================================================

static void print(const char* text)
{
    for (int i = 0; text[i] != '\0'; i++)
        put_char(text[i]);
}


// ============================================================
// PRINT UINT64
// ============================================================

static void print_uint(uint64_t number)
{
    if (number == 0)
    {
        put_char('0');
        return;
    }

    char buffer[32];
    int i = 0;

    while (number > 0)
    {
        buffer[i++] =
            '0' + (number % 10);

        number /= 10;
    }

    while (i > 0)
        put_char(buffer[--i]);
}


// ============================================================
// STRING COMPARE
// ============================================================

static bool string_equals(
    const char* a,
    const char* b
)
{
    int i = 0;

    while (a[i] != '\0' &&
           b[i] != '\0')
    {
        if (a[i] != b[i])
            return false;

        i++;
    }

    return a[i] == '\0' &&
           b[i] == '\0';
}


// ============================================================
// CLEAR SCREEN
// ============================================================

static void clear_screen()
{
    for (int i = 0; i < 80 * 25; i++)
        vga[i] = 0x0F00 | ' ';

    cursor_x = 0;
    cursor_y = 0;
}


// ============================================================
// MEMORY COMMAND
// ============================================================

static void show_memory()
{
    uint64_t total =
        Memory::get_total_memory();

    uint64_t used =
        Memory::get_used_memory();

    uint64_t free_memory =
        Memory::get_free_memory();

    print("\nMemory Usage\n");
    print("------------\n");

    print("Total: ");
    print_uint(total / (1024 * 1024));
    print(" MB\n");

    print("Used:  ");
    print_uint(used / (1024 * 1024));
    print(" MB\n");

    print("Free:  ");
    print_uint(free_memory / (1024 * 1024));
    print(" MB\n");
}


// ============================================================
// COMMAND EXECUTION
// ============================================================

static void execute_command()
{
    command_buffer[command_length] = '\0';

    put_char('\n');


    // --------------------------------------------------------
    // HELP
    // --------------------------------------------------------

    if (string_equals(command_buffer, "help"))
    {
        print("Available commands:\n");

        print("  help   - Show this help\n");
        print("  clear  - Clear the screen\n");
        print("  about  - About AzuredLinux\n");
        print("  echo   - Print text\n");
        print("  mem    - Show memory usage\n");
    }


    // --------------------------------------------------------
    // CLEAR
    // --------------------------------------------------------

    else if (string_equals(command_buffer, "clear"))
    {
        clear_screen();
    }


    // --------------------------------------------------------
    // ABOUT
    // --------------------------------------------------------

    else if (string_equals(command_buffer, "about"))
    {
        print("AzuredLinux v0.1\n");
        print("A hobby operating system made from scratch.\n");
        print("Architecture: x86_64\n");
    }


    // --------------------------------------------------------
    // MEMORY
    // --------------------------------------------------------

    else if (string_equals(command_buffer, "mem"))
    {
        show_memory();
    }


    // --------------------------------------------------------
    // ECHO
    // --------------------------------------------------------

    else if (command_length >= 5 &&
             command_buffer[0] == 'e' &&
             command_buffer[1] == 'c' &&
             command_buffer[2] == 'h' &&
             command_buffer[3] == 'o' &&
             command_buffer[4] == ' ')
    {
        for (int i = 5;
             i < command_length;
             i++)
        {
            put_char(command_buffer[i]);
        }

        put_char('\n');
    }


    // --------------------------------------------------------
    // UNKNOWN COMMAND
    // --------------------------------------------------------

    else if (command_length != 0)
    {
        print("Unknown command.\n");
    }


    // --------------------------------------------------------
    // RESET COMMAND BUFFER
    // --------------------------------------------------------

    command_length = 0;

    print("AzuredLinux> ");
}


// ============================================================
// SHELL INIT
// ============================================================

void shell_init()
{
    clear_screen();

    print("AzuredLinux Shell v0.1\n");
    print("Type 'help' for available commands.\n\n");

    print("AzuredLinux> ");
}


// ============================================================
// KEYBOARD INPUT
// ============================================================

void shell_put_char(char c)
{
    // ENTER
    if (c == '\n')
    {
        execute_command();
        return;
    }


    // BACKSPACE
    if (c == '\b')
    {
        if (command_length > 0)
        {
            command_length--;

            put_char('\b');
        }

        return;
    }


    // COMMAND BUFFER LIMIT
    if (command_length >= 127)
        return;


    // STORE CHARACTER
    command_buffer[command_length++] = c;

    // DISPLAY CHARACTER
    put_char(c);
}
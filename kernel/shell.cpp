#include "shell.hpp"
#include <stdint.h>

static volatile uint16_t* vga =
    (volatile uint16_t*)0xB8000;

static int cursor_x = 0;
static int cursor_y = 1;

static char command_buffer[128];
static int command_length = 0;

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
            vga[cursor_y * 80 + cursor_x] = 0x0F00 | ' ';
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

static void print(const char* text)
{
    for (int i = 0; text[i] != '\0'; i++)
        put_char(text[i]);
}

static bool string_equals(const char* a, const char* b)
{
    int i = 0;

    while (a[i] != '\0' && b[i] != '\0')
    {
        if (a[i] != b[i])
            return false;

        i++;
    }

    return a[i] == '\0' && b[i] == '\0';
}

static void clear_screen()
{
    for (int i = 0; i < 80 * 25; i++)
        vga[i] = 0x0F00 | ' ';

    cursor_x = 0;
    cursor_y = 0;
}

static void execute_command()
{
    command_buffer[command_length] = '\0';

    put_char('\n');

    if (string_equals(command_buffer, "help"))
    {
        print("Available commands:\n");
        print("  help\n");
        print("  clear\n");
        print("  about\n");
        print("  echo\n");
    }
    else if (string_equals(command_buffer, "clear"))
    {
        clear_screen();
    }
    else if (string_equals(command_buffer, "about"))
    {
        print("AzuredLinux v0.1\n");
        print("A hobby operating system made from scratch.\n");
    }
    else if (command_length >= 5 &&
             command_buffer[0] == 'e' &&
             command_buffer[1] == 'c' &&
             command_buffer[2] == 'h' &&
             command_buffer[3] == 'o' &&
             command_buffer[4] == ' ')
    {
        for (int i = 5; i < command_length; i++)
            put_char(command_buffer[i]);

        put_char('\n');
    }
    else if (command_length != 0)
    {
        print("Unknown command.\n");
    }

    command_length = 0;

    print("AzuredLinux> ");
}

void shell_init()
{
    clear_screen();

    print("AzuredLinux Shell v0.1\n");
    print("Type 'help' for available commands.\n\n");

    print("AzuredLinux> ");
}

void shell_put_char(char c)
{
    if (c == '\n')
    {
        execute_command();
        return;
    }

    if (c == '\b')
    {
        if (command_length > 0)
        {
            command_length--;
            put_char('\b');
        }

        return;
    }

    if (command_length >= 127)
        return;

    command_buffer[command_length++] = c;

    put_char(c);
}
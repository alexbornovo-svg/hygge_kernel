#include "../code/libs/kstd.h"
#include "../code/drivers/kbd.h"
#include "../code/utils/gdt.h"
#include "../code/utils/idt.h"
#include "../code/drivers/paging.h"
#include "../code/drivers/ata.h"
#include "../code/progfiles/test.h"
#include "../code/mm/kmalloc.h"
#include "string.h"

void kmain(void)
{
    uint line = 0;
    clearscreen();

    line = put_string(line, "Initializing GDT... ", WHITE);
    init_gdt();
    line = put_string(line, "[OK]", GREEN); 
    line = put_string(line, "Initializing IDT... ", WHITE);
    init_idt();
    line = put_string(line, "[OK]", GREEN);
    line = put_string(line, "Initializing ATA...", WHITE);
    line = init_ata(line);
    line = put_string(line, "[OK]", GREEN);
    line = put_string(line, "Initializing Paging... ", WHITE);
    init_paging();
    line = put_string(line, "[OK]", GREEN);
    line = put_string(line, "Initializing KMalloc...", WHITE);
    kmalloc_init();
    line = put_string(line, "[OK]", GREEN);
    line++;

    line = put_string(line, "Welcome to Hygge Kernel!", WHITE);
    line++;

    char in_buffer[64];

    while (1)
    {
        for (int i = 0; i < 64; i++) in_buffer[i] = 0;

        line = get_string(line, "> ", WHITE, in_buffer, sizeof(in_buffer));

        string_t input = string_form(in_buffer, sizeof(in_buffer));
        string_t ping  = STRING_LIT("ping");
        string_t cls = STRING_LIT("cls");
        string_t test = STRING_LIT("test");

        if (string_eq(ping, input))
        {
            line = put_string(line, "pong", WHITE);
            line++;
        }
        else if (string_eq(cls, input))
        {
            clearscreen();
            line = 0;
        }
        else if (string_eq(test, input))
        {
            line = run_tests(line);
        }
        else
        {
            line = put_string(line, "[ERROR] Undeclared command", RED);
        }

        // Scrolling
        if (line >= VGA_HEIGHT)
        {
            scroll_screen();
            line = VGA_HEIGHT - 1;
        }
    }
}
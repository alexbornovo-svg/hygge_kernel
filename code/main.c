#include "libs/kstd.h"
#include "drivers/kbd.h"
#include "utils/gdt.h"
#include "utils/idt.h"
#include "drivers/paging.h"
#include "string.h"

void kmain(void)
{
    uint line = 0;
    clearscreen();

    line = put_string(line, "Initializing GDT... ", WHITE);
    init_gdt();
    line = put_string(line, "[OK]", GREEN); 
    line = put_string(line, "Initializing IDT... ", WHITE);
    line = put_string(line, "[OK]", GREEN);
    line = put_string(line, "Initializing Paging... ", WHITE);
    line = put_string(line, "[OK]", GREEN);

    line = put_string(line, "Welcome to Hygge Kernel!", WHITE);
    line++;

    char in_buffer[64];

    while (1)
    {
        for (int i = 0; i < 64; i++) in_buffer[i] = 0;

        line = get_string(line, "> ", WHITE, in_buffer, sizeof(in_buffer));

        string_t input = string_form(in_buffer, sizeof(in_buffer));
        string_t ping  = STRING_LIT("ping");

        if (string_eq(ping, input))
        {
            line = put_string(line, "pong", WHITE);
            line++;
        }

        // Previene l'overflow dello schermo VGA (80x25)
        if (line >= 24)
        {
            clearscreen();
            line = 0;
        }
    }
}
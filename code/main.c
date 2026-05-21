#include "../code/libs/kstd.h"
#include "../code/drivers/kbd.h"

void kmain(void)
{
    uint line = 0;
    
    line = put_char(line,0, "C", WHITE, BLACK);
    clearscreen();

    line = 0;
    line = put_string(line, "Hygge Kernel", WHITE);

    uint col = 0;
    while (1)
    {
        char c = get_char();
        if (c != 0)
        {
            if (c == '\n' || col >= 80)
            {
                line++;
                col = 0;
            }
            else
            {
                put_char(line, col, c, WHITE, BLACK);
                col++;
            }
        }
    }
}
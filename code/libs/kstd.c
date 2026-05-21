#include "kstd.h"
#include "types.h"

ushort vga_entry(char c, uchar fg, uchar bg)
{
    return (ushort)c | (ushort)((bg & 0xF) << 4 | (fg & 0xF)) << 8;
}


uint put_char(uint line, uint col, char c, uchar fg, uchar bg)
{
    if (line >= VGA_HEIGHT || col >= VGA_WIDTH)
    {
        return (uint)-1;
    }

    uint pos = line * VGA_WIDTH + col;
    VGA_ADDR[pos] = vga_entry(c, fg, bg);
}

void clearscreen(void)
{
    char *vgamem = (char *) 0xb8000;
    uint index = 0;
    uint memdim = VGA_HEIGHT * VGA_WIDTH * 2;
    while (index < memdim)
    {
        vgamem[index] = ' ';
        index++;

        vgamem[index] = WHITE;
        index++;
    }
}

uint put_string(uint line, char *msg, uchar fg)
{
    char *vidmem = (char *) 0xb8000;

    uint index = 0;
    index = (line*80*2);

	while(*msg!=0)
	{
		if(*msg=='\n')
		{
			line++;
			index = (line*80*2);
			*msg++;
		} 
        else 
        {
			vidmem[index] = *msg;
			*msg++;
			index++;
			vidmem[index] = fg;
			index++;
		}
	}
}

char get_string(uint line, char *promt, uchar fg)
{
    char msg[16];
    uint col = 0;

    while (1)
    {
        char c = get_char();
        if (c != 0)
        {
            if (c == '\n' || col >= 80)
            {
                return msg;
            }
            else
            {
                put_char(line, col, c, fg, BLACK);
                col++;
            }
        }
    }
}
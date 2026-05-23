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
    
    return line + 1;
}


/*
line: starting point
prompt: printed before the input
fg: color
buf: output
buf_size: buffer dimension
*/
uint get_string(uint line, char *prompt, uchar fg, char *buf, uint buf_size)
{
    uint col = 0;
    uint p = 0;
    while (prompt[p] != '\0')
    {
        put_char(line, col, prompt[p], fg, BLACK);
        col++;
        p++;
    }

    uint start_col = col;

    while (1)
    {
        char c = get_char();
        if (c != 0)
        {
            if (c == '\b' && col > start_col)
            {
                col--;
                put_char(line, col, ' ', fg, BLACK);
                buf[col - start_col] = '\0';
            }
            else if (c == '\n' || (col - start_col) >= buf_size - 1)
            {
                buf[col - start_col] = '\0';
                return line + 1;
            }
            else
            {
                buf[col - start_col] = c;
                put_char(line, col, c, fg, BLACK);
                col++;
            }
        }
    }
}
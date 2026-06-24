#include "kstd.h"
#include "types.h"
#include "../drivers/kbd.h"

#include <stdarg.h> // printf behaviour

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

    return pos;
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


void scroll_screen()
{
    uint16_t* vga = VGA_MEMORY;

    // Sposta ogni riga in alto di una posizione
    for (int row = 1; row < VGA_HEIGHT; row++)
    {
        for (int col = 0; col < VGA_WIDTH; col++)
        {
            vga[(row - 1) * VGA_WIDTH + col] = vga[row * VGA_WIDTH + col];
        }
    }

    // Pulisce l'ultima riga
    for (int col = 0; col < VGA_WIDTH; col++)
    {
        vga[(VGA_HEIGHT - 1) * VGA_WIDTH + col] = (uint16_t)(WHITE << 8) | ' ';
    }
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

void kmemcpy(void *dst, const void *src, uint32_t n)
{
    uint8_t *d = (uint8_t *)dst;
    const uint8_t *s = (const uint8_t *)src;
    for (uint32_t i = 0; i < n; i++)
    {
        d[i] = s[i];
    }
}

void kmemset(void *dst, uint8_t val, uint32_t n)
{
    uint8_t *d = (uint8_t *)dst;
    for (uint32_t i = 0; i < n; i++)
    {
        d[i] = val;
    }
}

int kmemcmp(const void *a, const void *b, uint32_t n)
{
    const uint8_t *pa = (const uint8_t *)a;
    const uint8_t *pb = (const uint8_t *)b;
    for (uint32_t i = 0; i < n; i++)
    {
        if (pa[i] != pb[i]) return (int)pa[i] - (int)pb[i];
    }
    return 0;
}

static void uint32_to_hex(char *buf, uint32_t val)
{
    const char hex[] = "0123456789ABCDEF";
    buf[0] = '0'; buf[1] = 'x';
    for (int i = 7; i >= 0; i--)
    {
        buf[2 + i] = hex[val & 0xF];
        val >>= 4;
    }
    buf[10] = '\0';
}

static void uint32_to_dec(char *buf, uint32_t val)
{
    if (val == 0) { buf[0] = '0'; buf[1] = '\0'; return; }
    char tmp[12];
    int i = 0;
    while (val > 0) { tmp[i++] = '0' + (val % 10); val /= 10; }
    int j = 0;
    for (int k = i - 1; k >= 0; k--) buf[j++] = tmp[k];
    buf[j] = '\0';
}


void put_fmt(uint line, uint8_t color, const char *fmt, ...)
{
    char out[256];
    int  o = 0;
    va_list args;
    va_start(args, fmt);

    for (int i = 0; fmt[i] && o < 254; i++)
    {
        if (fmt[i] != '%') { out[o++] = fmt[i]; continue; }
        i++;
        char tmp[12];
        switch (fmt[i])
        {
            case 'x': case 'X':
                uint32_to_hex(tmp, va_arg(args, uint32_t));
                for (int k = 0; tmp[k] && o < 254; k++) out[o++] = tmp[k];
                break;
            case 'd': case 'u':
                uint32_to_dec(tmp, va_arg(args, uint32_t));
                for (int k = 0; tmp[k] && o < 254; k++) out[o++] = tmp[k];
                break;
            case 'c':
                out[o++] = (char)va_arg(args, int);
                break;
            case 's':
            {
                const char *s = va_arg(args, const char*);
                while (*s && o < 254) out[o++] = *s++;
                break;
            }
            case '%':
                out[o++] = '%';
                break;
            default:
                out[o++] = '%';
                out[o++] = fmt[i];
                break;
        }
    }
    out[o] = '\0';
    va_end(args);
    put_string(line, out, color);
}
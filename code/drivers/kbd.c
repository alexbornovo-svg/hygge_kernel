#include "types.h"
#include "kbd.h"
#include "io.h"
#include "../misc/kbd_ita.h"
#include "../libs/kstd.h"

uchar kbd_wait()
{
    return inb(0x64) & 0x01;
}

char scode_ascii(uchar scancode)
{
    if (scancode >= 128) return 0;
    return keymap_ita[scancode];
}

uchar read_sc()
{
    while ((inb(STAT_PORT) & 0x01) == 0) // Pause
    {
        __asm__ volatile("pause");
    }

    return inb(DATA_PORT);
}

char get_char()
{
    uchar scancode = read_sc();

    if (scancode & 0x80)
    {
        return 0;
    }

    if (scancode < sizeof(keymap_ita))
    {
        return scode_ascii(scancode);
    }

    return 0;
}
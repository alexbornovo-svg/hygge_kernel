#ifndef KSTD_H
#define KSTD_H

#include "types.h"

#define VGA_ADDR ((volatile ushort *)0xB8000)
#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define VGA_MEMORY ((uint16_t*)0xB8000)

#define WHITE 0xF
#define GREY 0x7
#define RED 0x4
#define GREEN 0x2
#define BLACK 0x0

uint put_char(uint line, uint col, char c, uchar fg, uchar bg);
uint put_string(uint line, char *msg, uchar fg);

uint get_string(uint line, char *prompt, uchar fg, char *buf, uint buf_size);

void clearscreen();

void scroll_screen();

#endif
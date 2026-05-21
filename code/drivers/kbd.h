#ifndef KBD_H
#define KBD_H

#include "types.h"

#define DATA_PORT 0x60
#define STAT_PORT 0x64

uint kbd_test(uint line);
char get_char();

#endif
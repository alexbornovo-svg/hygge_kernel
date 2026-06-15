#include "../include/io.h"
#include "pit.h"

void init_pit(uint32_t frequency) 
{
    uint32_t divisor = 1193182 / frequency; // 1.19 Mhz
    outb(0x43, 0x36); // square waves
    outb(0x40, (uint8_t)(divisor & 0xFF)); // low byte
    outb(0x40, (uint8_t)((divisor >> 8) & 0xFF)); // high byte
}
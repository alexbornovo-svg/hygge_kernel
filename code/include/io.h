#ifndef IO_H
#define IO_H

#include "types.h"

// Out

static inline void outb(ushort port, uchar val)
{
    __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

// In

static inline uchar inb(ushort port)
{
    uchar val;
    __asm__ volatile ("inb %1, %0" : "=a"(val) : "Nd"(port));
    return val;
}

// Utils

static inline void io_wait(void)
{
    __asm__ volatile ("outb %%al, $0x80" : : "a"(0));
}

#endif
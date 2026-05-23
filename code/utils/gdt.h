#ifndef GDT_H
#define GDT_H

#include "types.h"

struct gdt_entry 
{
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t  base_middle;
    uint8_t  access;
    uint8_t  granularity;
    uint8_t  base_high;
} __attribute__((packed));

struct gdt_ptr 
{
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

extern struct gdt_entry gdt[3];
extern struct gdt_ptr gp;

void gdt_set_gate(int num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran);
void init_gdt();

extern void gdt_flush(void);

#endif
#ifndef IDT_H
#define IDT_H

#include "types.h"
#include "regs.h"

#define IRQ_BASE 32

struct idt_entry
{
    uint16_t base_low;
    uint16_t selector;
    uint8_t  zero;
    uint8_t  flags;
    uint16_t base_high;
} __attribute__((packed));

struct idt_ptr
{
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

extern struct idt_entry idt[256];
extern struct idt_ptr   idtp;

void idt_set_gate(uint8_t num, uint32_t base, uint16_t selector, uint8_t flags);
void init_idt(void);
void irq_install(uint8_t irq, void (*handler)(registers_t *));
void isr_handler(registers_t *regs);
void irq_handler(registers_t *regs);
extern void idt_flush(void);

#endif
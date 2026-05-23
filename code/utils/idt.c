#include "types.h"
#include "idt.h"
#include "../code/libs/kstd.h"

struct idt_entry idt[256];
struct idt_ptr   idtp;

// isr :)
extern void isr0(void);  extern void isr1(void);  extern void isr2(void);
extern void isr3(void);  extern void isr4(void);  extern void isr5(void);
extern void isr6(void);  extern void isr7(void);  extern void isr8(void);
extern void isr9(void);  extern void isr10(void); extern void isr11(void);
extern void isr12(void); extern void isr13(void); extern void isr14(void);
extern void isr15(void); extern void isr16(void); extern void isr17(void);
extern void isr18(void); extern void isr19(void); extern void isr20(void);
extern void isr21(void); extern void isr22(void); extern void isr23(void);
extern void isr24(void); extern void isr25(void); extern void isr26(void);
extern void isr27(void); extern void isr28(void); extern void isr29(void);
extern void isr30(void); extern void isr31(void);

void idt_set_gate(uint8_t num, uint32_t base, uint16_t selector, uint8_t flags)
{
    idt[num].base_low  = base & 0xFFFF;
    idt[num].base_high = (base >> 16) & 0xFFFF;
    idt[num].selector  = selector;
    idt[num].zero      = 0;
    idt[num].flags     = flags;
}

void init_idt(void)
{
    idtp.limit = (sizeof(struct idt_entry) * 256) - 1;
    idtp.base  = (uint32_t)&idt;

    for (int i = 0; i < 256; i++)
    {
        idt_set_gate(i, 0, 0, 0);
    }

    // CPU exeptions
    idt_set_gate(0,  (uint32_t)isr0,  0x08, 0x8E);
    idt_set_gate(1,  (uint32_t)isr1,  0x08, 0x8E);
    idt_set_gate(2,  (uint32_t)isr2,  0x08, 0x8E);
    idt_set_gate(3,  (uint32_t)isr3,  0x08, 0x8E);
    idt_set_gate(4,  (uint32_t)isr4,  0x08, 0x8E);
    idt_set_gate(5,  (uint32_t)isr5,  0x08, 0x8E);
    idt_set_gate(6,  (uint32_t)isr6,  0x08, 0x8E);
    idt_set_gate(7,  (uint32_t)isr7,  0x08, 0x8E);
    idt_set_gate(8,  (uint32_t)isr8,  0x08, 0x8E);
    idt_set_gate(9,  (uint32_t)isr9,  0x08, 0x8E);
    idt_set_gate(10, (uint32_t)isr10, 0x08, 0x8E);
    idt_set_gate(11, (uint32_t)isr11, 0x08, 0x8E);
    idt_set_gate(12, (uint32_t)isr12, 0x08, 0x8E);
    idt_set_gate(13, (uint32_t)isr13, 0x08, 0x8E);
    idt_set_gate(14, (uint32_t)isr14, 0x08, 0x8E); // Page fault (very helpful i hope)
    idt_set_gate(15, (uint32_t)isr15, 0x08, 0x8E);
    idt_set_gate(16, (uint32_t)isr16, 0x08, 0x8E);
    idt_set_gate(17, (uint32_t)isr17, 0x08, 0x8E);
    idt_set_gate(18, (uint32_t)isr18, 0x08, 0x8E);
    idt_set_gate(19, (uint32_t)isr19, 0x08, 0x8E);
    idt_set_gate(20, (uint32_t)isr20, 0x08, 0x8E);
    idt_set_gate(21, (uint32_t)isr21, 0x08, 0x8E);
    idt_set_gate(22, (uint32_t)isr22, 0x08, 0x8E);
    idt_set_gate(23, (uint32_t)isr23, 0x08, 0x8E);
    idt_set_gate(24, (uint32_t)isr24, 0x08, 0x8E);
    idt_set_gate(25, (uint32_t)isr25, 0x08, 0x8E);
    idt_set_gate(26, (uint32_t)isr26, 0x08, 0x8E);
    idt_set_gate(27, (uint32_t)isr27, 0x08, 0x8E);
    idt_set_gate(28, (uint32_t)isr28, 0x08, 0x8E);
    idt_set_gate(29, (uint32_t)isr29, 0x08, 0x8E);
    idt_set_gate(30, (uint32_t)isr30, 0x08, 0x8E);
    idt_set_gate(31, (uint32_t)isr31, 0x08, 0x8E);

    idt_flush();
}

static void kpanic(uint line, const char *reason, const char *details)
{
    line = put_string(line, "*** KERNEL PANIC ***", RED);
    line++;
    line = put_string(line, (char *)reason,  WHITE);
    line++;
    if (details)
    {
        line = put_string(line, (char *)details, WHITE);
        line++;
    }
    for (;;) __asm__ volatile("hlt");
}

// Dispatch
void isr_handler(registers_t *regs)
{
    uint line = 0;

    if (regs->int_no == 14)
    {
        uint32_t fault_addr;
        __asm__ volatile("mov %%cr2, %0" : "=r"(fault_addr));

        uint8_t present = !(regs->err_code & 0x1);
        uint8_t rw      =  (regs->err_code & 0x2);
        uint8_t user    =  (regs->err_code & 0x4);

        if (present)
        {
            kpanic(line, "Page Fault: page not present", RED);
        }
        else if (rw)
        {
            kpanic(line, "Page Fault: write on read-only page", RED);
        }
        else if (user)
        {
            kpanic(line, "Page Fault: user mode violation", RED);
        }
        else 
        {
            kpanic(line, "Page Fault: unknown", RED);
        }
    }

    kpanic(line, "Unhandled CPU exception", RED);
}
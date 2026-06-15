#include "../code/utils/idt.h"
#include "../code/drivers/kbd.h"
#include "../code/libs/kstd.h"
#include "../code/mm/kmalloc.h"
#include "../code/utils/task.h"
#include "types.h"

/*
Batteria di test per GDT, IDT, Paging, IRQ, KMalloc
*/

static uint test_line = 0;

void task1(void)
{
    while (1)
    {
        __asm__ volatile("nop");
    }
}

void task2(void)
{
    while (1)
    {
        __asm__ volatile("nop");
    }
}

static uint test_pass(const char *name, uint test_line)
{
    test_line = put_string(test_line, (char *)name, WHITE);
    test_line = put_string(test_line, " [PASS]", GREEN);
    return test_line;
}

static uint test_fail(const char *name, uint test_line)
{
    test_line = put_string(test_line, (char *)name, WHITE);
    test_line = put_string(test_line, " [FAIL]", RED);
    return test_line;
}

/* ------------------------------------------------------------------ */
/*  1. GDT — legge i segment register, devono essere 0x10             */
/* ------------------------------------------------------------------ */
static uint test_gdt(uint test_line)
{
    uint16_t ds;
    __asm__ volatile("mov %%ds, %0" : "=r"(ds));
    if (ds == 0x10)
        test_line = test_pass("GDT: data segment", test_line);
    else
        test_line = test_fail("GDT: data segment", test_line);

    return test_line;
}

/* ------------------------------------------------------------------ */
/*  2. IDT — verifica che IDTR punti alla nostra tabella              */
/* ------------------------------------------------------------------ */
static uint test_idt(uint test_line)
{
    struct { uint16_t limit; uint32_t base; } __attribute__((packed)) idtr;
    __asm__ volatile("sidt %0" : "=m"(idtr));

    /* limit deve essere 256*8-1 = 2047 */
    if (idtr.limit == 2047 && idtr.base != 0)
        test_line = test_pass("IDT: IDTR loaded", test_line);
    else
        test_line = test_fail("IDT: IDTR loaded", test_line);

    return test_line;
}

/* ------------------------------------------------------------------ */
/*  3. Paging — CR0 bit 31 deve essere 1                              */
/* ------------------------------------------------------------------ */
static uint test_paging(uint test_line)
{
    uint32_t cr0;
    __asm__ volatile("mov %%cr0, %0" : "=r"(cr0));
    if (cr0 & 0x80000000)
        test_line = test_pass("Paging: CR0.PG set", test_line);
    else
        test_line = test_fail("Paging: CR0.PG set", test_line);

    return test_line;
}

/* ------------------------------------------------------------------ */
/*  4. IRQ — PIC remappato: legge IMR master, non deve essere 0xFF    */
/*     (0xFF = tutto mascherato, segnale che init_pic non è girato)   */
/* ------------------------------------------------------------------ */
static uint test_pic(uint test_line)
{
    /* legge IMR del PIC master (porta 0x21) inline */
    uint8_t imr;
    __asm__ volatile("inb $0x21, %0" : "=a"(imr));
    /* dopo pic_init le maschere sono quelle salvate prima del remapping,
       quasi certamente != 0xFF su un sistema che ha già eseguito pic_init */
    if (imr != 0xFF)
        test_line = test_pass("PIC: master IMR not 0xFF", test_line);
    else
        test_line = test_fail("PIC: master IMR not 0xFF", test_line);

    return test_line;
}

static uint test_kmalloc(uint test_line)
{
    // Calcoliamo la dimensione dell'header allineata (uguale a quella in kmalloc.c)
    // Se struct BlockHeader ha 2 puntatori/interi e un byte, occupa 12 byte, allineata a 8 = 16 byte.
    // Sostituisci 16 con la dimensione reale se differisce, o esporta la macro dal tuo .h
    const uint32_t HEADER_SIZE = 16; 

    void *a = kmalloc(64);
    void *b = kmalloc(64);

    /* CON LISTE CONCATENATE: b deve essere 64 byte + HEADER_SIZE dopo a */
    if (a != (void*)0 && b == (char*)a + 64 + HEADER_SIZE)
        test_line = test_pass("kmalloc: block allocation", test_line);
    else
        test_line = test_fail("kmalloc: block allocation", test_line);

    /* kmalloc_aligned deve restituire indirizzo multiplo di 4096 */
    void *c = kmalloc_aligned(128);
    if (((uint32_t)c & 0xFFF) == 0 && c != (void*)0)
        test_line = test_pass("kmalloc: aligned allocation", test_line);
    else
        test_line = test_fail("kmalloc: aligned allocation", test_line);

    /* NUOVO TEST: Verifica l'efficacia di kfree */
    void *d = kmalloc(32);
    kfree(d);
    void *e = kmalloc(32);
    
    // Se kfree funziona, 'e' dovrebbe riutilizzare lo stesso blocco appena liberato da 'd'
    if (d == e && d != (void*)0)
        test_line = test_pass("kmalloc: dynamic kfree reuse", test_line);
    else
        test_line = test_fail("kmalloc: dynamic kfree reuse", test_line);

    return test_line;
}

static uint test_scheduler(uint test_line)
{
    test_line = put_string(test_line, "Scheduler: init + task creation...", WHITE);

    init_scheduler();

    task_create(task1);
    task_create(task2);

    task_t* a = get_current_task();
    task_t* b;

    if (a != (void *)0)
    {
        test_line = put_string(test_line, "Scheduler: first task OK", GREEN);
    }
    else
    {
        return test_fail("Scheduler: first task", test_line);
    }

    scheduler_next((registers_t*)0);
    b = get_current_task();

    if (b != (void *)0 && b != a)
    {
        test_line = test_pass("Scheduler: context switch", test_line);
    }
    else
    {
        test_line = test_fail("Scheduler: context switch", test_line);
    }

    return test_line;
}

/* ------------------------------------------------------------------ */
/*  5. Page Fault intenzionale — divide by zero (INT 0) per testare   */
/*     che l'IDT catturi le eccezioni senza triple fault               */
/*     ATTENZIONE: questo test causa KERNEL PANIC — abilitalo solo    */
/*     commentando/decommentando manualmente                           */
/* ------------------------------------------------------------------ */
static uint test_page_fault(uint test_line)
{
    test_line = put_string(test_line, "IDT: triggering divide-by-zero...", WHITE);
    test_line++;
    __asm__ volatile("mov $0, %ecx; div %ecx");
    test_fail("IDT: divide-by-zero NOT caught", test_line);
}
uint run_tests(uint start_line)
{
    test_line = start_line;
    test_line = put_string(test_line, "--- Hygge Kernel Self Test ---", WHITE);
    test_line = test_gdt(test_line);
    test_line = test_idt(test_line);
    test_line = test_paging(test_line);
    test_line = test_pic(test_line);
    test_line = test_kmalloc(test_line);
    test_line = test_scheduler(test_line);
    test_line = put_string(test_line, "--- Tests done ---", WHITE);
    return test_line;
}


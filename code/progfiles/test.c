#include "../code/utils/idt.h"
#include "../code/drivers/kbd.h"
#include "../code/libs/kstd.h"
#include "../code/mm/kmalloc.h"
#include "../code/utils/task.h"
#include "../code/drivers/ata.h"
#include "../code/fs/ext2.h"
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
/*  ATA — verifica che il drive sia stato rilevato e che la lettura   */
/*        del settore 0 (MBR) restituisca la firma 0x55AA             */
/* ------------------------------------------------------------------ */
static uint test_ata(uint test_line)
{
    // 1. Drive presente dopo init_ata
    if (ata_active_drive.present)
        test_line = test_pass("ATA: drive detected", test_line);
    else
        return test_fail("ATA: drive detected", test_line);

    // 2. Lettura MBR: i byte 510-511 devono essere 0x55 0xAA
    uint16_t mbr[256];
    ata_read_sector(0, mbr);
    uint8_t *raw = (uint8_t*)mbr;
    /*
    if (raw[510] == 0x55 && raw[511] == 0xAA)
        test_line = test_pass("ATA: MBR signature 0x55AA", test_line);
    else
        test_line = test_fail("ATA: MBR signature 0x55AA", test_line);
    */
    // 3. Write+Read round-trip su un settore sicuro (LBA 1 = secondo settore,
    //    subito dopo l'MBR, prima del filesystem — non distrugge nulla su ext2
    //    che parte da LBA 2)
    uint16_t write_buf[256];
    uint16_t read_buf[256];
    for (int i = 0; i < 256; i++) write_buf[i] = (uint16_t)(i ^ 0xABCD);

    ata_write_sector(1, write_buf);
    ata_read_sector(1, read_buf);

    int ok = 1;
    for (int i = 0; i < 256; i++)
        if (read_buf[i] != write_buf[i]) { ok = 0; break; }

    if (ok)
        test_line = test_pass("ATA: write/read round-trip LBA1", test_line);
    else
        test_line = test_fail("ATA: write/read round-trip LBA1", test_line);

    return test_line;
}

/* ------------------------------------------------------------------ */
/*  EXT2 — verifica magic, lettura root inode, e listing root dir     */
/* ------------------------------------------------------------------ */
static uint test_ext2(uint test_line)
{
    // 1. Magic number nel superblock
    if (my_fs.sb != (void*)0 && my_fs.sb->ext2_magic == 0xEF53)
        test_line = test_pass("EXT2: superblock magic 0xEF53", test_line);
    else
        return test_fail("EXT2: superblock magic 0xEF53", test_line);

    // 2. Campi superblock plausibili
    int sb_ok = (my_fs.block_size >= 1024)
             && (my_fs.block_size <= 4096)
             && (my_fs.blocks_per_group > 0)
             && (my_fs.inodes_per_group > 0)
             && (my_fs.total_groups > 0);
    if (sb_ok)
        test_line = test_pass("EXT2: superblock fields sane", test_line);
    else
        test_line = test_fail("EXT2: superblock fields sane", test_line);

    // 3. BGD table: inode_table del gruppo 0 deve essere > 0
    if (my_fs.bgds != (void*)0 && my_fs.bgds[0].inode_table > 0)
        test_line = test_pass("EXT2: BGD group 0 inode_table > 0", test_line);
    else
        test_line = test_fail("EXT2: BGD group 0 inode_table > 0", test_line);

    // 4. Root inode (inode 2): deve essere una directory
    inode_t root;
    ext2_read_inode(ROOT_INODE_NUMBER, &root);
    if ((root.permission & 0xF000) == EXT2_S_IFDIR)
        test_line = test_pass("EXT2: root inode is directory", test_line);
    else
        test_line = test_fail("EXT2: root inode is directory", test_line);

    // 5. Root inode ha almeno un blocco dati
    if (root.blocks[0] != 0)
        test_line = test_pass("EXT2: root inode has data block", test_line);
    else
        test_line = test_fail("EXT2: root inode has data block", test_line);

    // 6. Prima direntry nella root ha inode != 0 (tipicamente ".")
    uint8_t block_buf[1024];
    ext2_read_block(root.blocks[0], block_buf);
    direntry_t *first = (direntry_t*)block_buf;
    if (first->inode != 0 && first->name_len > 0)
        test_line = test_pass("EXT2: root first direntry valid", test_line);
    else
        test_line = test_fail("EXT2: root first direntry valid", test_line);

    // 7. Prima direntry deve essere "." (inode == ROOT_INODE_NUMBER)
    if (first->inode == ROOT_INODE_NUMBER
        && first->name_len == 1
        && first->name[0] == '.')
        test_line = test_pass("EXT2: root direntry[0] is \".\"", test_line);
    else
        test_line = test_fail("EXT2: root direntry[0] is \".\"", test_line);

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
    /*
    test_line = test_gdt(test_line);
    test_line = test_idt(test_line);
    test_line = test_paging(test_line);
    test_line = test_pic(test_line);
    test_line = test_kmalloc(test_line);
    test_line = test_scheduler(test_line);
    */
    test_line = test_ata(test_line);
    test_line = test_ext2(test_line);
    test_line = put_string(test_line, "--- Tests done ---", WHITE);
    return test_line;
}
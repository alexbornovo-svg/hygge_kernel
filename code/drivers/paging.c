#include "types.h"
#include "paging.h"

// Static table 4KB
static uint32_t page_directory[1024] __attribute__((aligned(4096)));
static uint32_t page_table_0[1024]   __attribute__((aligned(4096)));

void init_paging(void)
{
    // map of first 4KB
    for (int i = 0; i < 1024; i++)
    {
        // phiscal addres
        page_table_0[i] = (i * PAGE_SIZE) | 0x3;
    }

    // insert in directory
    page_directory[0] = ((uint32_t)page_table_0) | 0x3;

    // not present entry
    for (int i = 1; i < 1024; i++)
    {
        page_directory[i] = 0x2;
    }

    switch_page_directory(page_directory);
}

void switch_page_directory(uint32_t *dir)
{
    __asm__ volatile(
        "mov %0, %%cr3\n" // page dir in CR3
        "mov %%cr0, %%eax\n"
        "or $0x80000000, %%eax\n"
        "mov %%eax, %%cr0\n" // PG bit active
        :
        : "r"(dir)
        : "eax"
    );
}
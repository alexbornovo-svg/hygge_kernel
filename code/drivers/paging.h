#ifndef PAGING_H
#define PAGING_H

#include "types.h"

#define PAGE_TABLE_ENTRIES 1024
#define PAGE_DIRECTORY_ENTRIES 1024
#define PAGE_SIZE 4096 // man I love 4KB, it is just perfect...

typedef struct page_entry
{
    uint32_t present : 1;
    uint32_t rw : 1;
    uint32_t user : 1;
    uint32_t accessed : 1;
    uint32_t dirty : 1;
    uint32_t unused : 7;
    uint32_t frame : 20;
} __attribute__((packed)) page_t;

typedef struct page_table 
{
    page_t pages[PAGE_TABLE_ENTRIES];
} page_table_t;

typedef struct page_directory 
{
    uint32_t tables_physical[PAGE_DIRECTORY_ENTRIES];
} page_directory_t;


void init_paging();

/*
Inizializza il paging, crea la directory e mappa il kernel (Identity Mapping)
    void init_paging();

Carica la page directory nel registro CR3 e attiva il bit di paging in CR0
    void switch_page_directory(page_directory_t *dir);

Gestore del Page Fault (l'interrupt che scatta se accedi a memoria non mappata)
    void page_fault_handler(void *registers);
*/

#endif
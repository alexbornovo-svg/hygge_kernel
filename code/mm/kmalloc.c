#include "types.h"
#include "kmalloc.h"

extern uint32_t _kernel_end;

typedef struct BlockHeader {
    uint32_t size;
    uint8_t  is_free;
    struct BlockHeader* next;
} BlockHeader_t;

#define HEADER_SIZE ((sizeof(BlockHeader_t) + 7) & ~7)

static BlockHeader_t* heap_start_block = (void *)0;
static uint32_t total_used_memory = 0;
static uint32_t heap_max_address = 0;

void kmalloc_init(void)
{
    uint32_t first_address = ((uint32_t)&_kernel_end + 0xFFF) & ~0xFFF;
    
    heap_start_block = (BlockHeader_t*)first_address;
    heap_start_block->size = 0; // Inizialmente vuoto
    heap_start_block->is_free = 1;
    heap_start_block->next = (void *)0;

    heap_max_address = first_address + HEADER_SIZE;
    total_used_memory = 0;
}

void *kmalloc(uint32_t size)
{
    if (size == 0) return (void *)0;

    size = (size + 7) & ~7;

    BlockHeader_t* current = heap_start_block;
    BlockHeader_t* best_fit = (void *)0;

    while (current != (void *)0) {
        if (current->is_free && current->size >= size) {
            best_fit = current;
            break;
        }
        if (current->next == (void *)0) break;
        current = current->next;
    }

    if (best_fit != (void *)0) 
    {
        best_fit->is_free = 0;
        total_used_memory += best_fit->size;
        return (void*)((uint32_t)best_fit + HEADER_SIZE);
    }

    BlockHeader_t* new_block = (BlockHeader_t*)heap_max_address;
    new_block->size = size;
    new_block->is_free = 0;
    new_block->next = (void *)0;

    if (current != heap_start_block || heap_start_block->size != 0) 
    {
        current->next = new_block;
    } 
    else 
    {
        heap_start_block = new_block;
    }

    heap_max_address += HEADER_SIZE + size;
    total_used_memory += size;

    return (void*)((uint32_t)new_block + HEADER_SIZE);
}

void kfree(void *ptr)
{
    if (ptr == (void *)0) return;

    BlockHeader_t* header = (BlockHeader_t*)((uint32_t)ptr - HEADER_SIZE);
    
    if (header->is_free == 0) {
        header->is_free = 1;
        total_used_memory -= header->size;
    }

    BlockHeader_t* current = heap_start_block;
    while (current != (void *)0 && current->next != (void *)0) 
    {
        if (current->is_free && current->next->is_free) 
        {
            current->size += HEADER_SIZE + current->next->size;
            current->next = current->next->next;
        } 
        else 
        {
            current = current->next;
        }
    }
}

void *kmalloc_aligned(uint32_t size)
{
    if (size == 0) return (void *)0;

    size = (size + 7) & ~7;

    uint32_t current_user_ptr = heap_max_address + HEADER_SIZE;
    uint32_t aligned_user_ptr = (current_user_ptr + 0xFFF) & ~0xFFF;

    uint32_t new_block_address = aligned_user_ptr - HEADER_SIZE;

    BlockHeader_t* current = heap_start_block;
    while (current != (void *)0 && current->next != (void *)0) {
        current = current->next;
    }

    BlockHeader_t* new_block = (BlockHeader_t*)new_block_address;
    new_block->size = size;
    new_block->is_free = 0;
    new_block->next = (void *)0;

    if (current != heap_start_block || heap_start_block->size != 0) {
        current->next = new_block;
    } else {
        heap_start_block = new_block;
    }
    heap_max_address = aligned_user_ptr + size;
    total_used_memory += size;

    return (void*)aligned_user_ptr;
}

uint32_t kmalloc_used(void)
{
    return total_used_memory;
}
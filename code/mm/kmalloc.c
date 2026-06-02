#include "types.h"
#include "kmalloc.h"

extern uint32_t _kernel_end;

static uint32_t heap_current = 0;
static uint32_t heap_start = 0;

void kmalloc_init(void)
{
    heap_start = ((uint32_t)&_kernel_end + 0xFFF) & ~0xFFF;
    heap_current = heap_start;
}

void *kmalloc(uint32_t size)
{
    if (size == 0)
    {
        return (void *)0;
    }
    size = (size + 7) & ~7;

    void *ptr     = (void *)heap_current;
    heap_current += size;
    return ptr;
}

void *kmalloc_aligned(uint32_t size)
{
    heap_current = (heap_current + 0xFFF) & ~0xFFF;
    return kmalloc(size);
}

void kfree(void *ptr)
{
    (void)ptr;
}

uint32_t kmalloc_used(void)
{
    return heap_current - heap_start;
}
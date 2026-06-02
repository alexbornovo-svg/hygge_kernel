#ifndef KMALLOC_H
#define KMALLOC_H
#include "types.h"

void  kmalloc_init(void);
void *kmalloc(uint32_t size);
void *kmalloc_aligned(uint32_t size);
void  kfree(void *ptr);                 /* no-op */
uint32_t kmalloc_used(void);            /* byte allocated */

#endif
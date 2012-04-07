#ifndef __KMALLOC_H__
#define __KMALLOC_H__

#include "defs.h"

void __kmem_init_kmalloc();

void __kmem_kmalloc_tests();

void* __kmalloc(uint32 size);
void __kfree(void *);

#endif

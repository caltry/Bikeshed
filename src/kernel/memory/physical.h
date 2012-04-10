#ifndef __PHYSICAL_MEMORY_H__
#define __PHYSICAL_MEMORY_H__

//#define KERNEL_LINK_ADDR 0xC0100000
#define KERNEL_LINK_ADDR 0x00010000
//#define KERNEL_LINK_ADDR 0x00100000
#define KERNEL_LOAD_ADDR 0x00010000

// Assume 256MB of RAM for now
#define PHYSICAL_MEM_SIZE 256*1024*1024

#include "defs.h"

//////////////////////////////
extern unsigned int KERNEL_END;

uint32 KERNEL_SIZE;

void __phys_set_bit(void* address);
void __phys_unset_bit(void* address);
uint32 __phys_check_bit(void* address);
void* __phys_get_free_4k(void);

void __phys_initialize_bitmap(void);

#endif

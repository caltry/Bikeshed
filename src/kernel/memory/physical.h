#ifndef __PHYSICAL_MEMORY_H__
#define __PHYSICAL_MEMORY_H__

#define KERNEL_LINK_ADDR 0xC0100000
#define KERNEL_LOAD_ADDR 0x00100000

#define ONE_KILOBYTE  (1024)
#define ONE_MEGABYTE  (1024 * ONE_KILOBYTE) 
#define SIXTEEN_MEG   (16 * ONE_MEGABYTE)
#define SIXTY_FOUR_KB (64 * ONE_KILOBYTE) 

#include "headers.h"

//////////////////////////////
extern Uint32 KERNEL_END;

Uint32 KERNEL_SIZE;
Uint32 PHYSICAL_MEM_SIZE;

/* Takes a physical address and sets the bit in the
 * bitmap. Doesn't do anything more than that
 */
void __phys_set_bit(void* address);

/* Takes a physical address and unsets the bit in the
 * bitmap. Doesn't do anything more than that
 */
void __phys_unset_bit(void* address);

/* Check if an address has already been allocated
 */
Uint32 __phys_check_bit(void* address);

/* Search the bitmap for an unused address. If there
 * are no free addresses then it causes a kernel panic
 */
void* __phys_get_free_4k(void);

/* Setup the physical memory bitmap. It marks the bottom
 * 1MiB of memory as unusable and then from it marks the
 * region KERNEL_LOAD_ADDR+KERNEL_SIZE as used.
 *
 * Note: This method adjusts KERNEL_SIZE to include the
 *       bitmap
 */
void __phys_initialize_bitmap(void);

/* Get the number of bits/4096 blocks of memory that are
 * free
 */
Uint32 __phys_get_free_page_count(void);
#endif

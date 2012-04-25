#ifndef __KMALLOC_H__
#define __KMALLOC_H__

#include "headers.h"

/* Initialize the kernel virtual memory allocator. 
 *
 * The kernel heap starts at 0xD0000000 and ends at 0xE0000000.
 * This gives 256MiB of kernel heap space. The kernel heap is
 * initialized to 8KiB and automatically grows to 0xE0000000 and
 * once full will kernel panic.
 */
void __kmem_init_kmalloc(void);

/* Allocate some memory off the heap and return a virtual
 * address to be used for whatever purpose.
 *
 * The minimum allocation size is 12 bytes because the header
 * structure needed by the free list is 12 bytes
 *
 * The allocated amount is also 4-byte aligned so the free
 * list structure is aligned. 
 *
 * The overhead per allocated block of memory is 4-bytes which
 * is the size field of the free list structure. This is needed
 * for krealloc() and makes combining free fragments easier.
 *
 * Free list:
 * +-------------+
 * | Size        |
 * +-------------+
 * | Next        |
 * +-------------+
 * | Previous    |
 * +-------------+
 * | Open Memory |
 * | ...         |
 * +-------------+
 *
 * Allocated memory:
 * +-------------+
 * | Size        |
 * +-------------+
 * | Open Memory | <- Returned pointer
 * | ...         |
 * +-------------+
 *
 * If there are no blocks that can be allocated another page
 * is requested from the physical memory allocator and added
 * to the end of the heap. If this causes the heap to go beyond
 * the maximum heap size a kernel panic happens.
 */
void* __kmalloc(Uint32 size);

/* Same as kmalloc, except zeros the memory before returning
 * a pointer to the memory
 */
void* __kcalloc(Uint32 size);

/* Print some info about the status of the kernel heap
 */
void __kmalloc_info(void);

/* Free a block of memory allocated by kmalloc(). There 
 * are no safe guards against bad blocks being passed 
 * into free() and will corrupt the heap if done.
 *
 * The free list is a simple linked list data structure.
 * The list of free nodes are kept sorted by virtual address.
 * When a node is being placed back into the free list the
 * previous and the next nodes are checked to see if they can
 * be combined with the current node and if so a combined node 
 * is made. This is done in order to keep the fragmentation of
 * the heap down.
 */
void __kfree(void *address);

#endif

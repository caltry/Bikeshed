/*
 * Globally accessable kernel functions used at startup, such as mapping
 * ramdisks into memory.
 *
 * David Larsen <dcl9934@cs.rit.edu> May, 2012
 */

#ifndef _VFS_INIT
#define _VFS_INIT

#include "types.h"

/*
 * Initalize a ramdisk by marking the virtual addresses as occupied then
 * mapping the virtual address to the physical addres.
 *
 * size is specified in KiB
 */
void _init_himem_ramdisk(
        void* physical_address,
        void* virtual_address,
        Uint32 size );

/*
 * Prepare all of the ramdisks for mounting.
 */
void _init_all_ramdisks(void);

#endif // _VFS_INIT

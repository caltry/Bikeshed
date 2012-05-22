/*
 * Globally accessable kernel functions, such as mapping ramdisks into memory.
 *
 * David Larsen <dcl9934@cs.rit.edu> May, 2012
 */

#include "vfs_init.h"
#include "kernel/serial.h"
#include "kernel/lib/klib.h"

/*
#include "memory/physical.h"
*/
/*
 * Takes a physical address and sets the bit in the
 * bitmap. Doesn't do anything more than that
 */
void __phys_set_bit(void* address);

/*
#include "memory/paging.h"
*/
#define PAGE_SIZE 4096
#define PRESENT 	0x1
#define READ_WRITE 	0x2
#define USER 		0x4
#define WRITE_THRU	0x8
#define CACHE_DISABLE 0x10
void __virt_map_page(void *physical_addr, void *virtual_addr, Uint32 flags);


/*
 * Initalize a ramdisk by marking the virtual addresses as occupied then
 * mapping the virtual address to the physical addres.
 *
 * size is specified in KiB
 */
void _init_himem_ramdisk(
	void* physical_address,
	void* virtual_address,
	Uint32 size )
{
	// If we're below 1MiB, we're already memory mapped, so stop here
	if( (Uint32) virtual_address < 0x100000 )
	{
		return;
	}

	serial_printf( "physical_address: %x\n\r", physical_address);
	serial_printf( "virtual_address: %x\n\r", virtual_address);
	serial_printf( "size: %d\n\r", size );

	void* physAddressMax = physical_address + 1024 * size;
	void* virtAddressMax = virtual_address + 1024 * size;

	void *p, *v;

	// Mark the physical addresses as occupied
	// Map the virtual address to the physical addresses
	Int32 page_num = 1;
	for( p = physical_address, v = virtual_address;
		p < physAddressMax && v < virtAddressMax;
		p += PAGE_SIZE, v += PAGE_SIZE )
	{
		__phys_set_bit( p );
		__virt_map_page( p, v, READ_WRITE );
		serial_printf("Mapping page: %d - phys: %x - virt: %x\n", page_num, p, v);
		++page_num;
	}
}

void _init_all_ramdisks()
{
	_init_himem_ramdisk( (void*) RAMDISK_PHYS_LOCATION,
		(void*) RAMDISK_VIRT_LOCATION,
		RAMDISK_SIZE_KiB );
}

/*
** File:	bios.c
**
** Author:	Sean Congden
**
** Description:	Support functions to interact with the bios
*/

#define	__KERNEL__20113__

#include "types.h"
#include "c_io.h"
#include "lib/klib.h"
#include "memory/paging.h"

#include "bios.h"


/*
** PUBLIC GLOBAL VARIABLES
*/

// Pointers to the beginning and end of the real mode code (defined in linker)
extern void *__start_real;
extern void *__end_real;


/*
** PUBLIC FUNCTIONS
*/


/*
** _bios_init()
**
** Initialize the bios module
*/

void _bios_init(void) {
	
	/*
	** Copy the real mode code to its destination (below 1MB)
	*/

	// Determine the length of the code segment
	Uint32 length = ((Uint32)(&__end_real - &__start_real)) * 32;
	c_printf("Real mode code len: %x\n", length);

	// Create an identity mapping for the addresses
	//   where the real mode code is currently located
	for (Uint32 load_pos = 0; load_pos < length; load_pos += 4096) {
		__virt_map_page((void *)(REAL_LOAD_ADDRESS + load_pos),
			(void *)(REAL_LOAD_ADDRESS + load_pos), PG_READ_WRITE);
	}
	
	// Copy the real mode code
	_kmemcpy(&__start_real, (void *)REAL_LOAD_ADDRESS, length * sizeof(char));
	
	// Unmap the source location pages
	for (Uint32 load_pos = 0; load_pos < length; load_pos += 4096) {
		__virt_clear_page((void *)(REAL_LOAD_ADDRESS + load_pos));
	}
}

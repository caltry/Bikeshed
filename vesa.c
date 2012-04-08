/*
** File:	vesa.c
**
** Author:	Sean Congden
**
** Description:	VESA support routines
*/

#define	__KERNEL__20113__

#include "headers.h"

#include "vesa.h"

/*
** PRIVATE DEFINITIONS
*/

/*
** PRIVATE DATA TYPES
*/

/*
** PRIVATE GLOBAL VARIABLES
*/

/*
** PUBLIC GLOBAL VARIABLES
*/

/*
** PRIVATE FUNCTIONS
*/

/*
** PUBLIC FUNCTIONS
*/

void print_vesa_info(void) {
	struct vesa_controller_info* info =
		(struct vesa_controller_info*)((Uint32)VESA_INFO_ADDRESS);

	c_puts("VESA Controller Information:\n");
	c_printf("    Header:  %s\n", &(info->signature));
	c_printf("    Version: %x\n", info->version);
	c_printf("    Video Memory: %d 64KB blocks\n", info->totalMemory);
}
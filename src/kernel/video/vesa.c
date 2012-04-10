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
	c_printf("    Video Memory: %d 64KB blocks\n", info->total_memory);

	c_puts("    Modes: ");
	Uint16* mode = (Uint16*)((Uint32)(info->video_modes[0]));
	int count = 0;
	while( *mode != 0xFFFF ) {
		c_printf("%d, ", *mode);
		mode = (Uint16*)((Uint32)((Uint16)mode + 1));
		count++;
	}
	c_printf("\n      %d total modes\n", count);
}

void print_mode_info(int mode_num) {
	struct vesa_mode_info* info =
		(struct vesa_mode_info*)((Uint32)VESA_MODE_ADDRESS);

	c_printf("VESA Mode %d: ", mode_num);
	c_printf("    %dx%d\n", info->x_resolution, info->y_resolution);
}
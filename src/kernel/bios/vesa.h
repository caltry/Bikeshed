/*
** File:	vesa.h
**
** Author:	Sean Congden
**
** Description:	VESA support routines and bios calls
*/

#ifndef _VESA_H
#define _VESA_H

/*
** General (C and/or assembly) definitions
*/

#define VESA_INFO_ADDRESS	0x00004000
#define VESA_MODE_ADDRESS	(VESA_INFO_ADDRESS + 512)
#define VESA_MODE_TO_LOAD	261

#define GET_CONTROLLER_INFO			0x4F00
#define GET_MODE_INFO				0x4F01
#define SET_MODE					0x4F02


#ifndef __ASM__20113__

/*
** Start of C-only definitions
*/

typedef short VesaFarPtr;
 
struct vesa_controller_info {
	char signature[4];
	Int16 version;
	VesaFarPtr oem_string[2];
	Uint8 capabilities[4];
	VesaFarPtr video_modes[2];
	Uint8 total_memory;
};

struct vesa_mode_info {
	Uint16 attributes;
	Uint8 window_a;
	Uint8 window_b;
	Uint16 window_granularity;
	Uint16 window_size;
	Uint16 window_a_segment;
	Uint16 window_b_segment;
	VesaFarPtr window_function;
	Uint16 pitch;

	Uint16 x_resolution;
	Uint16 y_resolution;
	Uint8 x_char_size;
	Uint8 y_char_size;
	Uint8 num_planes;
	Uint8 bits_per_pixel;
	Uint8 num_banks;
	Uint8 memory_model;
	Uint8 bank_size;
	Uint8 num_img_pages;
	Uint8 reserved_0;

	Uint8 red_mask_size;
	Uint8 red_position;
	Uint8 green_mask_size;
	Uint8 green_position;
	Uint8 blue_mask_size;
	Uint8 blue_position;
	Uint8 rsvd_mask_size;
	Uint8 rsvd_position;
	Uint8 direct_color_attributes;

	Uint32 framebuffer_address;
	Uint32 reserved_1;
	Uint16 reserved_2;

	/* VBE 3.0+ and above... */
};

/*
** Types
*/

/*
** Globals
*/

/*
** Prototypes
*/

void print_vesa_info(void);
void print_mode_info(int mode_num);

#endif

#endif

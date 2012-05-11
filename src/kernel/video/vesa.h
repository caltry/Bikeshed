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

#define GET_CONTROLLER_INFO			0x4F00
#define GET_MODE_INFO				0x4F01
#define SET_MODE					0x4F02

/* Mode Attributes */
#define MODE_SUPPORTED				1
#define COLOR_MODE					8
#define GRAPHICS_MODE				16
#define LINEAR_BUFFER				128

#define REQUIRED_ATTRIBUTES			(MODE_SUPPORTED | COLOR_MODE \
										| GRAPHICS_MODE | LINEAR_BUFFER)

/* Mode Memory Model */
#define PACKED_PIXELS				4
#define DIRECT_COLOR				6


#define SUPPORTED_DEPTH(x)				(x == 24)


#ifndef __ASM__20113__

#include "headers.h"

/*
** Start of C-only definitions
*/

typedef short VesaFarPtr;
 
typedef struct vesa_controller_info {
	Uint32 signature;
	Int16 version;
	Uint32 oem_string;
	Uint32 capabilities;
	Uint32 video_modes;
	Uint16 total_memory;
} __attribute__((packed)) VesaControllerInfo;

typedef struct vesa_mode_info {
	Uint16 attributes;
	Uint8 window_a;
	Uint8 window_b;
	Uint16 window_granularity;
	Uint16 window_size;
	Uint16 window_a_segment;
	Uint16 window_b_segment;
	Uint32 window_function;
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

	Uint32 frame_buffer_address;
	Uint32 reserved_1;
	Uint16 reserved_2;

	/* VBE 3.0+ and above... */
} __attribute__((packed)) VesaModeInfo;

/*
** Types
*/

/*
** Globals
*/

/*
** Prototypes
*/

void _vesa_init(void);

void _vesa_load_info(VesaControllerInfo *info);
void _vesa_load_mode_info(Uint16 mode, VesaModeInfo *info);
Uint16 _vesa_choose_mode(Uint16 *modes, int x, int y);
void _vesa_select_mode(Uint16 mode);

void _vesa_print_info(VesaControllerInfo *info);
void _vesa_print_mode_info_basic(int mode_num, VesaModeInfo *info);
void _vesa_print_mode_info(int mode_num, VesaModeInfo *info);

#endif

#endif

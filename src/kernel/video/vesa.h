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
** General definitions
*/

/* Addresses to use when loading in VBE info via BIOS calls */
#define VESA_INFO_ADDRESS			0x00004000
#define VESA_MODE_ADDRESS			(VESA_INFO_ADDRESS + 512)


/*
** Start of C-only definitions
*/

#ifndef __ASM__20113__

#include "types.h"


/* VBE function codes */
#define GET_CONTROLLER_INFO			0x4F00
#define GET_MODE_INFO				0x4F01
#define SET_MODE					0x4F02

/* VBE mode attributes we need for a proper graphical display */
#define MODE_SUPPORTED				1
#define COLOR_MODE					8
#define GRAPHICS_MODE				16
#define LINEAR_BUFFER				128

#define REQUIRED_ATTRIBUTES			(MODE_SUPPORTED | COLOR_MODE \
										| GRAPHICS_MODE | LINEAR_BUFFER)

/* Mode Memory Model */
#define PACKED_PIXELS				4
#define DIRECT_COLOR				6

/* A list of supported bit depths bikeshed supports */
#define SUPPORTED_DEPTH(x)			((x == 24) || (x == 32))


/*
** Types
*/

/* VBE controller information */
typedef struct vesa_controller_info {
	Uint32 signature;
	Int16 version;
	Uint32 oem_string;
	Uint32 capabilities;
	Uint32 video_modes;
	Uint16 total_memory;
} __attribute__((packed)) VesaControllerInfo;

/* VBE info for an individual video mode */
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
** Prototypes
*/


/*
** _vesa_load_info(info)
**
** Loads VBE controller information into the provided struct
*/
void _vesa_load_info(VesaControllerInfo *info);


/*
** _vesa_load_mode_info(mode,info)
**
** Loads VBE video mode information into the provided struct
*/
void _vesa_load_mode_info(Uint16 mode, VesaModeInfo *info);


/*
** _vesa_choose_mode(mode,x,y)
**
** Iterates through the video modes available and chooses
** a supported video mode closest to the provided resolution
*/
Uint16 _vesa_choose_mode(Uint16 *mode, int x, int y);


/*
** _vesa_select_mode(mode)
**
** Sets the provided video mode as active.
*/
void _vesa_select_mode(Uint16 mode);


/*
** _vesa_print_info(info)
**
** Prints the provided VBE controller information
*/
void _vesa_print_info(VesaControllerInfo *info);


/*
** _vesa_load_info(mode_num,info)
**
** Prints information about the provided video mode
*/
void _vesa_print_mode_info(int mode_num, VesaModeInfo *info);


/*
** _vesa_print_mode_info_basic(info)
**
** Prints a summary of the provided video mode
*/
void _vesa_print_mode_info_basic(int mode_num, VesaModeInfo *info);


#endif

#endif

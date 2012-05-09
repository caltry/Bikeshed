/*
** File:	video.c
**
** Author:	Sean Congden
**
** Description:	Video / graphics module
*/

#define	__KERNEL__20113__

#include "headers.h"
#include "memory/kmalloc.h"

#include "vesa.h"

#include "graphics.h"

#include "video.h"


#define TRACE_VIDEO
#ifdef TRACE_VIDEO
#	define TRACE(...) c_printf(__VA_ARGS__)
#else
#	define TRACE(...) ;
#endif


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

Status _video_init(void) {
	//Uint16 video_mode;
	VesaControllerInfo *info = (VesaControllerInfo *)VESA_INFO_ADDRESS;
	_vesa_load_info(info);
	_vesa_print_info(info);

	//Uint16 *modes = (Uint16 *)(info->video_modes);
	//_vesa_choose_mode(modes, 1024, 768, 24);
	//_vesa_load_mode_info(video_mode);
	//_vesa_print_mode_info(video_mode);

	VesaModeInfo *mode = (VesaModeInfo *)VESA_MODE_ADDRESS;
	_vesa_load_mode_info(280, mode);
	
	// Collect details about the screen
	kScreen = __kmalloc(sizeof(Screen));
	kScreen->frame_buffer = (Uint16 *)(mode->frame_buffer_address);
	kScreen->width = mode->x_resolution;
	kScreen->height = mode->y_resolution;
	kScreen->bpp = mode->bits_per_pixel;
	kScreen->pitch = mode->pitch;
	kScreen->size = mode->y_resolution * mode->pitch;

	// Make sure pages are identity mapped for the screen
	for (Uint32 address = 0; address < kScreen->size; address += 2048) {
		__virt_map_page((void *)(kScreen->frame_buffer + address),
			(void *)(kScreen->frame_buffer + address), READ_WRITE | PRESENT);
	}

	// Switch to the mode
	_vesa_select_mode(280);

	// Create the default screen background
	clear_screen(kScreen, 0x6B8A8B);

	return SUCCESS;
}
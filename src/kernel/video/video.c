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
	Screen *screen = __kmalloc(sizeof(Screen));
	screen->frame_buffer = (Uint16 *)(mode->frame_buffer_address);
	screen->width = mode->x_resolution;
	screen->height = mode->y_resolution;
	screen->bpp = mode->bits_per_pixel;
	screen->pitch = mode->pitch;
	screen->size = mode->y_resolution * mode->pitch;

	// Make sure pages are identity mapped for the screen
	for (Uint32 address = 0; address < screen->size; address += 2048) {
		__virt_map_page((void *)(screen->frame_buffer + address),
			(void *)(screen->frame_buffer + address), READ_WRITE | PRESENT);
	}

	// Switch to the mode
	_vesa_select_mode(280);

	// Draw to the screen
	clear_screen(screen, 0xa066b4);
	fill_rect(screen, 50, 50, 1024 - 100, 768 - 100, 0xa3905f);
	
	//TODO: save somewhere
	//__kfree(screen);
	
	return SUCCESS;
}
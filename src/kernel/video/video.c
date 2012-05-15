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
#include "ulib.h"

#include "vesa.h"
#include "graphics.h"
#include "gconsole.h"

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
	// Load the vesa controller information
	VesaControllerInfo *info = (VesaControllerInfo *)VESA_INFO_ADDRESS;
	_vesa_load_info(info);
	_vesa_print_info(info);

	// Determine the video mode to use
	Uint16 *modes = (Uint16 *)(info->video_modes);
	Uint16 mode_num = _vesa_choose_mode(modes, 1024, 768);

	VesaModeInfo *mode = (VesaModeInfo *)VESA_MODE_ADDRESS;
	_vesa_load_mode_info(mode_num, mode);
	_vesa_print_mode_info(mode_num, mode);
	
	// Collect details about the screen
	kScreen = __kmalloc(sizeof(Screen));
	kScreen->frame_buffer = (Uint16 *)(mode->frame_buffer_address);
	kScreen->width = mode->x_resolution;
	kScreen->height = mode->y_resolution;
	kScreen->bpp = mode->bits_per_pixel;
	kScreen->pitch = mode->pitch;
	kScreen->size = mode->y_resolution * mode->pitch;

	// Set up the back buffer
	c_printf("size: %d\n", kScreen->size);
	kScreen->back_buffer = __kmalloc(kScreen->size);
	sem_init(kScreen->buffer_lock);
	sem_post(kScreen->buffer_lock);

	// Make sure pages are identity mapped for the screen
	for (Uint32 address = 0; address < kScreen->size; address += 2048) {
		__virt_map_page((void *)(kScreen->frame_buffer + address),
			(void *)(kScreen->frame_buffer + address), READ_WRITE | PRESENT);
	}

	// Switch to the mode
	_vesa_select_mode(mode_num);

	// Create the default screen background
	clear_screen(kScreen, 0x6B8A8B);

	return SUCCESS;
}


Uint16 *_video_aquire_buffer(Screen *screen) {
	//TODO: use read/write locks so multiple drawing
	//  operations can run at the same time
	//sem_wait(screen->buffer_lock);
	return screen->back_buffer;
}


void _video_release_buffer(Screen *screen) {
	//sem_post(screen->buffer_lock);
}


void _video_run(void) {
	/*
	** Start up the system graphics module
	*/
	_video_init();
	gconsole_flush();

	while( 1 ) {
		gconsole_draw(16, 16);

		//sem_wait(kScreen->buffer_lock);

		// Copy the back buffer to the screen
		_kmemcpy((void *)(kScreen->frame_buffer),
			(void *)(kScreen->back_buffer), kScreen->size);

		//sem_post(kScreen->buffer_lock);

		// Update at about 200 fps
		msleep(5);
	}
}
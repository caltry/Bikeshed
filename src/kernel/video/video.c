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
#include "gconsole.h"

#include "video.h"


Screen *kScreen;


Status _video_init(void) {
	// Load the vesa controller information
	VesaControllerInfo *info = (VesaControllerInfo *)VESA_INFO_ADDRESS;
	_vesa_load_info(info);
	_vesa_print_info(info);

	// Determine the video mode to use
	Uint16 *modes = (Uint16 *)(info->video_modes);
	Uint16 mode_num = _vesa_choose_mode(modes, 1024, 768);
	//Uint16 mode_num = 283;

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
	kScreen->back_buffer = __kmalloc(kScreen->size);
	sem_init(kScreen->buffer_lock);
	sem_post(kScreen->buffer_lock);

	// Make sure pages are identity mapped for the screen
	for (Uint32 address = 0; address < (kScreen->size + 4096); address += 4096) {
		//c_printf("mapping: %x to %x\n", (Uint32)((Uint32)(kScreen->frame_buffer) + address),
		//	(Uint32)((Uint32)FRAMEBUFFER_ADDRESS + address));

		__virt_map_page((void *)(Uint32)((Uint32)(kScreen->frame_buffer) + address),
			(void *)(Uint32)((Uint32)FRAMEBUFFER_ADDRESS + address), READ_WRITE | PRESENT);
	}
	//c_printf("screen size: %d", kScreen->size);

	kScreen->frame_buffer = (Uint16 *)((Uint32)FRAMEBUFFER_ADDRESS);

	// Switch to the mode
	_vesa_select_mode(mode_num);

	// Clear the background
	_kmemclr(kScreen->frame_buffer, kScreen->size);
	_kmemclr(kScreen->back_buffer, kScreen->size);

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

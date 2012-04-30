/*
** File:	graphics.c
**
** Author:	Sean Congden
**
** Description:	Basic drawing functions
*/

#define	__KERNEL__20113__

#include "headers.h"

#include "video.h"

#include "graphics.h"


#define TRACE_GRAPHICS
#ifdef TRACE_GRAPHICS
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

//TODO: fix!
static void fill_rect_16(Screen *screen, Uint32 x, Uint32 y, Uint32 width, Uint32 height, Uint32 color) {
	Uint16 *start = (Uint16 *)(screen->frame_buffer + (y * screen->pitch) + x);

	Uint8 r = (Uint8)(color >> 4);
	Uint8 g = (Uint8)((color & 0xff00) >> 2);
	Uint8 b = (Uint8)(color & 0xff);
	Uint16 fill = ((b >> 3) << 11) | ((g >> 2) << 5) | (r >> 3);

	for ( Uint32 y = 0; y < height; ++y ) {
		for ( Uint32 x = 0; x < width; ++x ) {
			start[x] = fill;
		}

		start = (Uint16 *)((Uint32)start + screen->pitch);
	}
}


static void fill_rect_24(Screen *screen, Uint32 x, Uint32 y, Uint32 width, Uint32 height, Uint32 color) {
	Uint8 *start = (Uint8 *)(screen->frame_buffer) + (y * screen->pitch) + (x * 3);

	Uint8 r = (Uint8)((color & 0xff0000) >> 16);
	Uint8 g = (Uint8)((color & 0x00ff00) >> 8);
	Uint8 b = (Uint8)(color & 0x0000ff);

	for ( Uint32 y = 0; y < height; ++y ) {
		Uint8 *dest = start;
		for ( Uint32 x = 0; x < width; ++x ) {
			dest[0] = b;
			dest[1] = g;
			dest[2] = r;

			dest += 3;
		}

		start = (Uint8 *)((Uint32)start + screen->pitch);
	}
}


/*
** PUBLIC FUNCTIONS
*/


void clear_screen(Screen *screen, Uint32 color) {
	Uint8 *dest = (Uint8 *)(screen->frame_buffer);
	Uint32 num = screen->size;

	Uint8 r = (Uint8)((color & 0xff0000) >> 16);
	Uint8 g = (Uint8)((color & 0x00ff00) >> 8);
	Uint8 b = (Uint8)(color & 0x0000ff);

	while( num -= 3 ) {
		dest[0] = b;
		dest[1] = g;
		dest[2] = r;

		dest += 3;
	}
}


void set_pixel(Screen *screen, Uint32 x, Uint32 y, Uint32 color) {

}


void fill_rect(Screen *screen, Uint32 x, Uint32 y, Uint32 width, Uint32 height, Uint32 color) {
	switch ( screen->bpp ) {
		case 16:
			fill_rect_16(screen, x, y, width, height, color);
	 		break;
		case 24:
			fill_rect_24(screen, x, y, width, height, color);
		default:
			break;
	}
}


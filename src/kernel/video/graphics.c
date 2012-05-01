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

#include "font.h"

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


void draw_char(Screen *screen, char letter, int x, int y, Uint32 color) {
	Uint8 *dest = (Uint8 *)(screen->frame_buffer) + (y * screen->pitch) + (x * 3);

	// Only uppercase letters for now
	if ((letter < 32) || (letter > 126)) return;
	Uint32 index = (letter - 32) * 7;

	Uint8 r = (Uint8)((color & 0xff0000) >> 16);
	Uint8 g = (Uint8)((color & 0x00ff00) >> 8);
	Uint8 b = (Uint8)(color & 0x0000ff);

	for (int row = 0; row < 7; ++row) {
		Uint8 row_data = font_data[index + row];

		for (int i = 0; i < 3; ++i) {
			for (int col = 0; col < 5; ++col) {
				if ((row_data << col) & 0x10) {
					dest[0] = dest[3] = dest[6] = b;
					dest[1] = dest[4] = dest[7] = g;
					dest[2] = dest[5] = dest[8] = r;
				}

				dest += 9;
			}

			dest = (Uint8 *)((Uint32)dest + screen->pitch - (9 * 5));
		}
	}
}


void draw_string(Screen *screen, char *string, int x, int y, Uint32 color) {
	char *cur = string;
	while (*cur != '\0') {
		draw_char(screen, *cur, x, y, color);
		x += (6 * 3);
		cur++;
	}
}


void test_pattern(Screen *screen) {
	Uint8 *dest = (Uint8 *)(screen->frame_buffer);
	Uint32 num = screen->size;

	while( num -= 3 ) {
		Uint32 shift = num % (screen->pitch + 1);

		if (shift % 30 > 15) {
			dest[0] = 0x00;
			dest[1] = 0xaa;
			dest[2] = 0xaa;
		} else {
			dest[0] = 0x22;
			dest[1] = 0x22;
			dest[2] = 0x22;
		}

		dest += 3;
	}
}


void set_pixel(Screen *screen, Uint32 x, Uint32 y, Uint32 color) {
	Uint8 *dest = (Uint8 *)(screen->frame_buffer);

	dest[0] = (Uint8)(color & 0x0000ff);
	dest[1] = (Uint8)((color & 0x00ff00) >> 8);
	dest[2] = (Uint8)((color & 0xff0000) >> 16);
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


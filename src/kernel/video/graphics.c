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


void fill_rect(Screen *screen, Uint32 x, Uint32 y, Uint32 width, Uint32 height, Uint32 color) {
	Uint8 *start = (Uint8 *)_video_aquire_buffer(screen) + (y * screen->pitch) + (x * 3);

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

	_video_release_buffer(screen);
}


/*
** PUBLIC FUNCTIONS
*/


void clear_screen(Screen *screen, Uint32 color) {
	Uint8 *dest = (Uint8 *)_video_aquire_buffer(screen);
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

	_video_release_buffer(screen);
}


void draw_char(Screen *screen, char letter, int x, int y, int scale, Uint32 color) {
	Uint8 *dest = (Uint8 *)_video_aquire_buffer(screen) + (y * screen->pitch) + (x * 3);

	// Only uppercase letters for now
	if ((letter < 32) || (letter > 126)) return;
	Uint32 index = (letter - 32) * 7;

	Uint8 r = (Uint8)((color & 0xff0000) >> 16);
	Uint8 g = (Uint8)((color & 0x00ff00) >> 8);
	Uint8 b = (Uint8)(color & 0x0000ff);

	for (int row = 0; row < 7; ++row) {
		Uint8 row_data = font_data[index + row];

		for (int i = 0; i < scale; ++i) {
			for (int col = 0; col < 5; ++col) {
				if ((row_data << col) & 0x10) {
					dest[0] = dest[3] = dest[6] = b;
					dest[1] = dest[4] = dest[7] = g;
					dest[2] = dest[5] = dest[8] = r;
				}

				dest += (scale * 3);
			}

			dest = (Uint8 *)((Uint32)dest + screen->pitch - (scale * 3 * 5));
		}
	}

	_video_release_buffer(screen);
}


void draw_string(Screen *screen, char *string, int x, int y, int scale, Uint32 color) {
	char *cur = string;
	while (*cur != '\0') {
		draw_char(screen, *cur, x, y, scale, color);
		x += (6 * scale);
		cur++;
	}
}


void draw_chars(Screen *screen, char *start, char *end, int x, int y, int scale, Uint32 color) {
	while (start != end) {
		draw_char(screen, *start, x, y, scale, color);
		x += (6 * scale);
		start++;
	}
}


void set_pixel(Screen *screen, Uint32 x, Uint32 y, Uint32 color) {
	Uint8 *dest = (Uint8 *)_video_aquire_buffer(screen);

	dest[0] = (Uint8)(color & 0x0000ff);
	dest[1] = (Uint8)((color & 0x00ff00) >> 8);
	dest[2] = (Uint8)((color & 0xff0000) >> 16);

	_video_release_buffer(screen);
}
/*
** File:	painter32.cpp
**
** Author:	Sean Congden
**
** Description:	Basic drawing functions
*/

extern "C" {
	#include "font.h"
}

#include "painter.h"
#include "painter32.h"


Painter32::Painter32(Screen *screen, Rect bounds)
	: Painter(screen, bounds)
{
}

Painter32::~Painter32(void)
{
}


void Painter32::Fill(Uint32 color) {
	Uint8 *dest = (Uint8 *)_video_aquire_buffer(screen);
	Uint32 num = screen->size;

	Uint8 r = (Uint8)((color & 0xff0000) >> 16);
	Uint8 g = (Uint8)((color & 0x00ff00) >> 8);
	Uint8 b = (Uint8)(color & 0x0000ff);

	while( num -= 4 ) {
		dest[0] = b;
		dest[1] = g;
		dest[2] = r;
		dest[3] = 0xff;

		dest += 4;
	}

	_video_release_buffer(screen);
}


void Painter32::FillRect(Rect clipped_rect, Uint32 color) {
	//Rect *clipped_rect = &bounds;//.Intersection(bounds + rect);

	Uint8 *start = (Uint8 *)_video_aquire_buffer(screen)
		 + (clipped_rect.y * screen->pitch) + (clipped_rect.x * 4);

	Uint8 r = (Uint8)((color & 0xff0000) >> 16);
	Uint8 g = (Uint8)((color & 0x00ff00) >> 8);
	Uint8 b = (Uint8)(color & 0x0000ff);

	for ( Uint32 y = 0; y < clipped_rect.height; ++y ) {
		Uint8 *dest = start;
		for ( Uint32 x = 0; x < clipped_rect.width; ++x ) {
			dest[0] = b;
			dest[1] = g;
			dest[2] = r;
			dest[3] = 0xff;

			dest += 4;
		}

		start = (Uint8 *)((Uint32)start + screen->pitch);
	}

	_video_release_buffer(screen);
}


void Painter32::DrawChar(char letter, int x, int y, int scale, Uint32 color) {
	// Limit the range to printable ascii characters
	if ((letter < 32) || (letter > 126)) return;

	Uint8 *dest = (Uint8 *)_video_aquire_buffer(screen) +
		(y * screen->pitch) + (x * 4);

	Uint32 index = (letter - 32) * 7;

	Uint8 r = (Uint8)((color & 0xff0000) >> 16);
	Uint8 g = (Uint8)((color & 0x00ff00) >> 8);
	Uint8 b = (Uint8)(color & 0x0000ff);

	for (int row = 0; row < 7; ++row) {
		Uint8 row_data = font_data[index + row];

		for (int i = 0; i < scale; ++i) {
			for (int col = 0; col < 5; ++col) {
				if ((row_data << col) & 0x10) {
					dest[0] = dest[4] = dest[8] = b;
					dest[1] = dest[5] = dest[9] = g;
					dest[2] = dest[6] = dest[10] = r;
					dest[3] = dest[7] = dest[11] = 0xff;
				}

				dest += (scale * 4);
			}

			dest = (Uint8 *)((Uint32)dest + screen->pitch - (scale * 4 * 5));
		}
	}

	_video_release_buffer(screen);
}


void Painter32::SetPixel(Uint32 x, Uint32 y, Uint32 color) {
	if (!bounds.Contains(bounds.x + x, bounds.y + y)) return;

	Uint8 *dest = (Uint8 *)_video_aquire_buffer(screen)
		+ ((bounds.y + y) * screen->pitch) + ((bounds.x + x) * 4);

	dest[0] = (Uint8)(color & 0x0000ff);
	dest[1] = (Uint8)((color & 0x00ff00) >> 8);
	dest[2] = (Uint8)((color & 0xff0000) >> 16);
	dest[3] = 0xff;

	_video_release_buffer(screen);
}


void Painter32::DrawCursor(Int32 mouse_x, Int32 mouse_y)
{
	Uint8 *start = (Uint8 *)(screen->frame_buffer)
		 + (mouse_y * screen->pitch) + (mouse_x * 4);

	for ( Uint32 y = 0; y < 8; ++y ) {
		Uint8 *dest = start;
		for ( Uint32 x = 0; x < 8; ++x ) {
			dest[0] = 0xff;//~dest[0];
			dest[1] = 0x00;//~dest[1];
			dest[2] = 0x00;//~dest[2];
			dest[4] = 0xff;

			dest += 4;
		}

		start = (Uint8 *)((Uint32)start + screen->pitch);
	}
}


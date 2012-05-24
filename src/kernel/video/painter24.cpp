/*
** File:	painter24.cpp
**
** Author:	Sean Congden
**
** Description:	Basic drawing functions
*/

extern "C" {
	#include "font.h"
}

#include "painter.h"
#include "painter24.h"


Painter24::Painter24(Screen *screen, Rect bounds)
	: Painter(screen, bounds)
{
}

Painter24::~Painter24(void)
{
}


void Painter24::Fill(Uint32 color) {
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


void Painter24::FillRect(Rect rect, Uint32 color) {
	Rect clipped_rect = rect.Clip(bounds);

	Uint8 *start = (Uint8 *)_video_aquire_buffer(screen)
		 + (clipped_rect.y * screen->pitch) + (clipped_rect.x * 3);

	Uint8 r = (Uint8)((color & 0xff0000) >> 16);
	Uint8 g = (Uint8)((color & 0x00ff00) >> 8);
	Uint8 b = (Uint8)(color & 0x0000ff);

	for ( Uint32 y = 0; y < clipped_rect.height; ++y ) {
		Uint8 *dest = start;
		for ( Uint32 x = 0; x < clipped_rect.width; ++x ) {
			dest[0] = b;
			dest[1] = g;
			dest[2] = r;

			dest += 3;
		}

		start = (Uint8 *)((Uint32)start + screen->pitch);
	}

	_video_release_buffer(screen);
}


void Painter24::DrawChar(char letter, int x, int y, int scale, Uint32 color) {
	Uint8 *dest = (Uint8 *)_video_aquire_buffer(screen) +
			(y * screen->pitch) + (x * 3);

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


void Painter24::SetPixel(Uint32 x, Uint32 y, Uint32 color) {
	if (!bounds.Contains(bounds.x + x, bounds.y + y)) return;

	Uint8 *dest = (Uint8 *)_video_aquire_buffer(screen)
		+ ((bounds.y + y) * screen->pitch) + ((bounds.x + x) * 3);

	dest[0] = (Uint8)(color & 0x0000ff);
	dest[1] = (Uint8)((color & 0x00ff00) >> 8);
	dest[2] = (Uint8)((color & 0xff0000) >> 16);

	_video_release_buffer(screen);
}


void Painter24::DrawCursor(Int32 mouse_x, Int32 mouse_y)
{
	Uint8 *start = (Uint8 *)(screen->frame_buffer)
		 + (mouse_y * screen->pitch) + (mouse_x * 3);

	for ( Uint32 y = 0; y < 8; ++y ) {
		Uint8 *dest = start;
		for ( Uint32 x = 0; x < 8; ++x ) {
			dest[0] = 0xff;//~dest[0];
			dest[1] = 0x00;//~dest[1];
			dest[2] = 0x00;//~dest[2];

			dest += 3;
		}

		start = (Uint8 *)((Uint32)start + screen->pitch);
	}
}


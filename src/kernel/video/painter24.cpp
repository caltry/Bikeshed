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


void Painter24::SetPixel(Int32 x, Int32 y, Uint32 color) {
	if (!bounds.Contains(bounds.x + x, bounds.y + y)) return;

	Uint8 *dest = (Uint8 *)_video_aquire_buffer(screen)
		+ ((bounds.y + y) * screen->pitch) + ((bounds.x + x) * 3);

	dest[0] = (Uint8)(color & 0x0000ff);
	dest[1] = (Uint8)((color & 0x00ff00) >> 8);
	dest[2] = (Uint8)((color & 0xff0000) >> 16);

	_video_release_buffer(screen);
}

#define CLAMP(x, min, max)	((x < min) ? min : (x > max) ? max : x)

void Painter24::DrawCursor(Int32 mouse_x, Int32 mouse_y)
{
	Uint32 x = CLAMP(mouse_x, bounds.x, bounds.x2 - 8);
	Uint32 y = CLAMP(mouse_y, bounds.y, bounds.y2 - 8);

	Uint8 *start = (Uint8 *)(screen->frame_buffer)
		 + (y * screen->pitch) + (x * 3);

	for ( Uint32 i = 0; i < 8; ++i ) {
		Uint8 *dest = start;
		for ( Uint32 j = 0; j < 8; ++j ) {
			dest[0] = 0xff;//~dest[0];
			dest[1] = 0x00;//~dest[1];
			dest[2] = 0x00;//~dest[2];

			dest += 3;
		}

		start = (Uint8 *)((Uint32)start + screen->pitch);
	}
}


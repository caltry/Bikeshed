/*
** File:	painter.cpp
**
** Author:	Sean Congden
**
** Description:	Basic drawing functions
*/

extern "C" {
	#include "video.h"
	#include "font.h"
}

#include "rect.h"

#include "painter.h"


Painter::Painter(Screen *screen, Rect bounds)
	: screen(screen)
	, bounds(bounds)
{
}


void Painter::SetClipping(Rect bounds)
{
	this->bounds = bounds;
}


void Painter::Fill(Uint32 color) {
	Uint8 *dest = (Uint8 *)_video_aquire_buffer(screen);
	//Uint8* start = dest;
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

	//c_printf("start: %x  dest: %x   diff: %d\n", start, dest, dest - start);

	_video_release_buffer(screen);
}


void Painter::FillRect(Rect clipped_rect, Uint32 color) {
	//Rect *clipped_rect = &bounds;//.Intersection(bounds + rect);

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


void Painter::DrawChar(char letter, int x, int y, int scale, Uint32 color) {
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


void Painter::DrawString(char *string, int x, int y, int scale, Uint32 color) {
	char *cur = string;
	while (*cur != '\0') {
		DrawChar(*cur, x, y, scale, color);
		x += (6 * scale);
		cur++;
	}
}


void Painter::DrawString(char *start, char *end, int x, int y, int scale, Uint32 color) {
	while (start != end) {
		DrawChar(*start, x, y, scale, color);
		x += (6 * scale);
		start++;
	}
}


void Painter::SetPixel(Uint32 x, Uint32 y, Uint32 color) {
	if (!bounds.Contains(bounds.x + x, bounds.y + y)) return;

	Uint8 *dest = (Uint8 *)_video_aquire_buffer(screen)
		+ ((bounds.y + y) * screen->pitch) + ((bounds.x + x) * 3);

	dest[0] = (Uint8)(color & 0x0000ff);
	dest[1] = (Uint8)((color & 0x00ff00) >> 8);
	dest[2] = (Uint8)((color & 0xff0000) >> 16);

	_video_release_buffer(screen);
}


void Painter::DrawBox(Rect& bounds, Uint32 color1, Uint32 color2,
	Uint32 color3, Uint32 color4, Uint32 scale)
{
	// The middle
	FillRect(Rect(bounds.x + scale, bounds.y + scale,
		bounds.width - (scale * 2), bounds.height - (scale * 2)), color1);

	// The top and bottom sides
	FillRect(Rect(bounds.x + scale, bounds.y,
		bounds.width - (scale * 2), scale), color2);
	FillRect(Rect(bounds.x + scale, bounds.y2 - scale,
		bounds.width - (scale * 2), scale), color2);

	// The left and right sides
	FillRect(Rect(bounds.x, bounds.y + scale, scale,
		bounds.height - (scale * 2)), color3);
	FillRect(Rect(bounds.x2 - scale, bounds.y + scale, scale,
		bounds.height - (scale * 2)), color3);

	// The four corners
	FillRect(Rect(bounds.x, bounds.y, scale, scale), color4);
	FillRect(Rect(bounds.x2 - scale, bounds.y, scale, scale), color4);
	FillRect(Rect(bounds.x, bounds.y2 - scale, scale, scale), color4);
	FillRect(Rect(bounds.x2 - scale, bounds.y2 - scale,
		scale, scale), color4);
}


void Painter::DrawCursor(Int32 mouse_x, Int32 mouse_y)
{
	Uint8 *start = (Uint8 *)(screen->frame_buffer)
		 + (mouse_y * screen->pitch) + (mouse_x * 3);

	for ( Uint32 y = 0; y < 8; ++y ) {
		Uint8 *dest = start;
		for ( Uint32 x = 0; x < 8; ++x ) {
			dest[0] = ~dest[0];
			dest[1] = ~dest[1];
			dest[2] = ~dest[2];

			dest += 3;
		}

		start = (Uint8 *)((Uint32)start + screen->pitch);
	}
}


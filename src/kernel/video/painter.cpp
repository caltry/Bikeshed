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


Painter::Painter(Screen *_screen, Rect _bounds)
	: screen(_screen)
	, bounds(_bounds)
{
}


Painter::~Painter(void)
{
}


void Painter::SetClipping(Rect bounds)
{
	this->bounds = bounds;
}


void Painter::DrawChar(char letter, int x, int y, int scale, Uint32 color) {
	// Limit the range to printable ascii characters
	if ((letter < 32) || (letter > 126)) return;
	Uint32 index = (letter - 32) * FONT_HEIGHT;

	int target_x = x;
	int target_y = y;

	for (int row = 0; row < FONT_HEIGHT; ++row) {
		Uint8 row_data = font_data[index + row];
	
		for (int col = 0; col < FONT_WIDTH; ++col) {
			for (int i = 0; i < scale; ++i) {
				if ((row_data << col) & 0x10) {
					for (int j = 0; j < scale; ++j) {
						SetPixel(x + col * scale + i,
							y + row * scale + j, color);
					}
				}
				target_x++;
			}

			target_x = x;
			target_y++;
		}
	}

	_video_release_buffer(screen);
}


void Painter::DrawString(char *string, int x, int y, int scale, Uint32 color) {
	if (y + FONT_HEIGHT > bounds.y2) return;

	char *cur = string;
	while ((*cur != '\0') && (x < bounds.x2) && (y < bounds.y2)) {
		if (((x + FONT_WIDTH * scale) > 0) && ((y + FONT_HEIGHT * scale) > 0))
			DrawChar(*cur, x, y, scale, color);
		
		x += ((FONT_WIDTH + 1) * scale);
		cur++;
	}
}


void Painter::DrawString(char *start, char *end, int x, int y, int scale, Uint32 color) {
	if (y + FONT_HEIGHT > bounds.y2) return;

	while ((start != end) && (x < bounds.x2) && (y < bounds.y2)) {
		if (((x + FONT_WIDTH * scale) > 0) && ((y + FONT_HEIGHT * scale) > 0))
			DrawChar(*start, x, y, scale, color);
		
		x += ((FONT_WIDTH + 1) * scale);
		start++;
	}
}


void Painter::DrawBox(Rect& bounds, Uint32 color1, Uint32 color2,
	Uint32 color3, Uint32 color4, Int32 scale)
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


/*
** File:	painter.cpp
**
** Author:	Sean Congden
**
** Description:	Basic drawing functions
*/

extern "C" {
	#include "video.h"
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


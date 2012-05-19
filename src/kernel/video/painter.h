/*
** File:	painter.h
**
** Author:	Sean Congden
**
** Description:	Basic drawing functions
*/

#ifndef _PAINTER_H
#define _PAINTER_H

extern "C" {
	#include "video.h"
}

#include "rect.h"

class Painter {
public:

	Painter(Screen *screen, Rect bounds);
	
	void SetClipping(Rect bounds);

	void Fill(Uint32 color);

	virtual void DrawChar(char letter, int x, int y, int scale = 3, Uint32 color = 0x0);
	void DrawString(char *string, int x, int y, int scale = 3, Uint32 color = 0x0);
	void DrawString(char *start, char *end, int x, int y, int scale = 3, Uint32 color = 0x0);

	virtual void SetPixel(Uint32 x, Uint32 y, Uint32 color);

	virtual void FillRect(Rect bounds, Uint32 color);

	void DrawBox(Rect& bounds, Uint32 color1, Uint32 color2,
		Uint32 color3, Uint32 color4, Uint32 scale);

	void DrawCursor(Int32 x, Int32 y);
	
protected:
	Screen *screen;
	Rect bounds;
};

#endif

/*
** File:	painter.h
**
** Author:	Sean Congden
**
** Description:	Basic drawing functions
*/

#ifndef _PAINTER32_H
#define _PAINTER32_H

#include "painter.h"

class Painter32 : public Painter {
public:

	Painter32(Screen *screen, Rect bounds);
	virtual ~Painter32(void);

	void SetPixel(Int32 x, Int32 y, Uint32 color);
	void FillRect(Rect bounds, Uint32 color);
	void Fill(Uint32 color);
	
	void DrawCursor(Int32 x, Int32 y);
};

#endif

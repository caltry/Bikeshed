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

	/*
	** Painter(screen,bounds)
	**
	** A painter provides drawing routines to create graphics within
	** the provided clip region.
	*/
	Painter(Screen *screen, Rect bounds);
	virtual ~Painter(void);

	/*
	** SetPixel(x,y,color)
	**
	** Sets the pixel at the given coordinates to the provided color.
	*/
	virtual void SetPixel(Int32 x, Int32 y, Uint32 color) = 0;
	
	/*
	** FillRect(bounds,color)
	**
	** Fills the provided rectangle bounds with the given color.
	*/
	virtual void FillRect(Rect bounds, Uint32 color) = 0;
	
	/*
	** SetPosition(x,y)
	**
	** Moves the top left corner of the rectangle to the
	** provided x and y coordinates.
	*/
	virtual void Fill(Uint32 color) = 0;

	/*
	** DrawCursor(x,y)
	**
	** Draws a mouse cursor directly to the framebuffer at the
	** provided x and y coordinates.
	*/
	virtual void DrawCursor(Int32 x, Int32 y) = 0;


	/*
	** DrawChar(letter,x,y,scale,color)
	**
	** Draws the provided character at the given x/y coordinates
	** font size, and font color.
	*/
	void DrawChar(char letter, int x, int y, int scale = 3, Uint32 color = 0);
	
	/*
	** DrawString(string,x,y,scale,color)
	**
	** Draws the provided string at the given coordinates,
	** scale, and font color.
	*/
	void DrawString(char *string, int x, int y, int scale = 3,
		Uint32 color = 0);
	
	/*
	** DrawString(start,end,x,y,scale,color)
	**
	** Draws a string from the start pointer to the end pointer at the
	** given coordinates, scale, and font color.
	*/
	void DrawString(char *start, char *end, int x, int y, int scale = 3,
		Uint32 color = 0);

	/*
	** DrawBox(bounds,color1,color2,color3,color4,scale)
	**
	** Draws a stylized box with the with the given bounds.
	** The colors describe the base color, the top/bottom bevel color,
	** the left/right bevel color, and the corner bevel color.
	*/
	void DrawBox(Rect& bounds, Uint32 color1, Uint32 color2,
		Uint32 color3, Uint32 color4, Int32 scale);
	

	/*
	** SetClipping(bounds)
	**
	** Restricts the drawing calls the the provided
	** region in screen coordinates.
	*/
	void SetClipping(Rect bounds);

protected:
	Screen *screen;
	Rect bounds;
};

#endif

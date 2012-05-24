/*
** File:	rect.h
**
** Author:	Sean Congden
**
** Description:	A basic rectangle
*/

#ifndef _RECT_H
#define _RECT_H

extern "C" {
	#include "types.h"
}

class Rect {
public:

	/*
	** Rect(x,y,width,height)
	**
	** A rectangle defined by the top left coordinate x and y
	** and its width and height.
	*/
	Rect(Int32 x, Int32 y, Uint32 width, Uint32 height);
	~Rect(void);

	/*
	** SetPosition(x,y)
	**
	** Moves the top left corner of the rectangle to the
	** provided x and y coordinates.
	*/
	void SetPosition(Int32 x, Int32 y);

	/*
	** Clip(other)
	**
	** Clips the current rectangle using the other rectangle as
	** the clip boundaries.
	*/
	Rect Clip(const Rect& other) const;

	/*
	** Intersects(rect)
	**
	** Indicates if the provided rectangle intersects the current rectangle.
	**
	** Returns:
	**      if the provided rectangle intersects
	*/
	bool Intersects(const Rect& other) const;

	/*
	** Contains(x,y)
	**
	** Indicates if the rectangle includes the provided coordinate
	**
	** Returns:
	**      if the provided coordinates are in the rectangle
	*/
	bool Contains(Int32 x, Int32 y) const;

	/*
	** Contains(x,y)
	**
	** Indicates if the rectangle completely contains the other rectangle
	**
	** Returns:
	**      if the rectangle encloses the provided rectangle
	*/
	bool Contains(const Rect& other) const;

	Rect& operator=(const Rect& other);
	bool operator==(const Rect& other) const;

	Int32 x, y, x2, y2;
	Uint32 width, height;
};

#endif

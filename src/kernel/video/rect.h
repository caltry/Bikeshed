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

	Rect(Int32 x, Int32 y, Uint32 width, Uint32 height);
	~Rect(void);

	void SetPosition(Int32 x, Int32 y);

	Rect Clip(const Rect& other) const;

	bool Intersects(const Rect& other) const;
	bool Contains(Int32 x, Int32 y) const;
	bool Contains(const Rect& other) const;

	Rect& operator=(const Rect& other);
	bool operator==(const Rect& other) const;

	Int32 x, y, x2, y2;
	Uint32 width, height;
};

#endif

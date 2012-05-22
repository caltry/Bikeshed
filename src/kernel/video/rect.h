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

	Rect(Uint32 x, Uint32 y, Uint32 width, Uint32 height);
	~Rect(void);

	void SetPosition(Uint32 x, Uint32 y);

	Rect* Intersection(const Rect& other) const;

	bool Intersects(const Rect& other) const;
	bool Contains(Uint32 x, Uint32 y) const;
	bool Contains(const Rect& other) const;

	Rect& operator=(const Rect& other);
	bool operator==(const Rect& other) const;
	Rect operator+(const Rect& other);

	Uint32 x, y, width, height;
	Uint32 x2, y2;
};

#endif

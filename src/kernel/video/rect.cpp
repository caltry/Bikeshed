/*
** File:	rect.cpp
**
** Author:	Sean Congden
**
** Description:	A basic rectangle
*/

#include "rect.h"


Rect::Rect(Uint32 x, Uint32 y, Uint32 width, Uint32 height)
	: x(x)
	, y(y)
	, width(width)
	, height(height)
	, x2(x + width)
	, y2(y + height)
{
}


Rect::~Rect(void)
{
}


void Rect::SetPosition(Uint32 x, Uint32 y)
{
	this->x = x;
	this->y = y;
	
	this->x2 = x + width;
	this->y2 = y + height;
}


bool Rect::Intersects(const Rect &other) const
{
	return !((this->x > other.x2)
		|| (other.x > this->x2)
		|| (this->y > other.y2)
		|| (other.y > this->y2));
	}


bool Rect::Contains(Uint32 x, Uint32 y) const
{
	return (x >= this->x) && (x <= this->x2)
		&& (y >= this->y) && (y <= this->y2);
}

bool Rect::Contains(const Rect& other) const
{
	return Contains(other.x, other.y)
		&& Contains(other.x2, other.y2);
}


#define CLAMP(x, min, max)	((x < min) ? min : (x > max) ? max : x)


Rect Rect::Clip(const Rect& other) const
{
	Uint32 x = CLAMP(this->x, other.x, other.x2);
	Uint32 y = CLAMP(this->y, other.y, other.y2);
	Uint32 x2 = CLAMP(this->x2, other.x, other.x2);
	Uint32 y2 = CLAMP(this->y2, other.y, other.y2);

	return Rect(x, y, x2 - x, y2 - y);
}


Rect& Rect::operator=(const Rect& other)
{
	if (&other == this)
		return *this;

	this->x = other.x;
	this->y = other.y;
	this->width = other.width;
	this->height = other.height;
	this->x2 = other.x2;
	this->y2 = other.y2;

	return *this;
}


bool Rect::operator==(const Rect& other) const
{
	return (this->x == other.x) && (this->y == other.y)
		&& (this->width == other.width) && (this->height == other.height);
}

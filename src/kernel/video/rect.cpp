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


#define MAX(x, y)		((x > y) ? x : y)
#define MIN(x, y)		((x < y) ? x : y)


Rect* Rect::Intersection(const Rect& other) const
{
	Uint32 x = MAX(this->x, other.x);
	Uint32 y = MAX(this->x, other.x);
	Uint32 x2 = MIN(this->x2, other.x2);
	Uint32 y2 = MIN(this->y2, other.y2);

	return new Rect(x, y, x2, y2);
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


Rect Rect::operator+(const Rect& other)
{
	return Rect(this->x + other.x, this->y + other.y, other.width, other.height);
}

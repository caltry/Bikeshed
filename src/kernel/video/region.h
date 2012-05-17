/*
** File:	region.h
**
** Author:	Sean Congden
**
** Description:	
*/

#ifndef _REGION_H
#define _REGION_H

extern "C" {
	#include "linkedlist.h"
}

class Region {
public:

	Region(void);
	~Region();

	void AddRect(Rect& rect);

	bool Intersects(const Region& region) const;
	bool Intersects(const Rect& rect) const;

	bool Contains(Uint32 x, Uint32 y);

	Region& operator=(const Region& other);

private:
	LinkedList *rect_list;
};

#endif

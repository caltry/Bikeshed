/*
** File:	region.h
**
** Author:	Sean Congden
**
** Description:	A data type that represents areas of screen space
*/

#ifndef _REGION_H
#define _REGION_H

extern "C" {
	#include "linkedlist.h"
}

class Region {
public:

	/*
	** Region()
	**
	** Regions represent areas of screen space as an aggregate of
	** of rectangles.
	*/
	Region(void);
	~Region();

	/*
	** AddRect(rect)
	**
	** Adds the area of the provided rectangle to the region.
	*/
	void AddRect(Rect& rect);

	/*
	** Intersects(region)
	**
	** Indicates if the provided region intersects the current region.
	**
	** Returns:
	**      if the provided region intersects
	*/
	bool Intersects(const Region& region) const;

	/*
	** Intersects(rect)
	**
	** Indicates if the provided rectangle intersects the current region.
	**
	** Returns:
	**      if the provided rectangle intersects
	*/
	bool Intersects(const Rect& rect) const;

	/*
	** Contains(x,y)
	**
	** Indicates if the region includes the provided coordinate
	**
	** Returns:
	**      if the provided coordinates are in the region
	*/
	bool Contains(Uint32 x, Uint32 y);


	Region& operator=(const Region& other);

private:
	LinkedList *rect_list;
};

#endif

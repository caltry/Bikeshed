/*
** File:	region.cpp
**
** Author:	Sean Congden
**
** Description:	
*/

extern "C" {
	#include "kmalloc.h"
}

#include "rect.h"

#include "region.h"


Region::Region(void)
{
	rect_list = (LinkedList *)__kmalloc(sizeof(LinkedList));
	list_init(rect_list, __kfree);
}


Region::~Region(void)
{
	list_destroy(rect_list);
}


void Region::AddRect(Rect& rect)
{
	Rect *data = new Rect(rect);

	list_insert_next(rect_list, NULL, (void *)data);
}


bool Region::Intersects(const Rect& rect) const
{
	bool intersects = false;

	ListElement* cur_node = list_head(rect_list);
	while (cur_node != NULL) {
		if (((Rect *)list_data(cur_node))->Intersects(rect)) {
			intersects = true;
			break;
		}

		cur_node = cur_node->next;
	}

	return intersects;
}


bool Region::Contains(Uint32 x, Uint32 y)
{
	bool contains = false;

	ListElement* cur_node = list_head(rect_list);
	while (cur_node != NULL) {
		if (((Rect *)list_data(cur_node))->Contains(x, y)) {
			contains = true;
			break;
		}

		cur_node = cur_node->next;
	}

	return contains;
}


Region& Region::operator=(const Region& other)
{
	if (&other == this)
		return *this;

	list_destroy(rect_list);
	list_init(rect_list, __kfree);

	ListElement* cur_node = list_head(other.rect_list);
	while (cur_node != NULL) {
		list_insert_next(rect_list, NULL,
			new Rect(*((Rect *)list_data(cur_node))));

		cur_node = cur_node->next;
	}

	return *this;
}

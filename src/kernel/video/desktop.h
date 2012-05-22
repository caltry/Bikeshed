/*
** File:	desktop.h
**
** Author:	Sean Congden
**
** Description:	
*/

#ifndef _DESKTOP_H
#define _DESKTOP_H

#ifdef __cplusplus

extern "C" {
	#include "linkedlist.h"
	#include "video.h"
}

#include "rect.h"

class Window;

class Desktop {
public:

	Desktop(Screen *screen);
	~Desktop(void);

	void AddWindow(Window *window);

	Rect& GetBounds(void) { return bounds; }

	void Draw(void);
	
private:
	Screen *screen;
	Rect bounds;
	Painter *painter;

	linked_list_t *window_list;

	Sem list_sem;
};

extern "C" {
#endif

	void _desktop_run(void);

#ifdef __cplusplus
}
#endif

#endif

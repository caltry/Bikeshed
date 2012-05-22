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
	Painter *GetPainter(void) { return painter; }

	void Draw(void);
	void DrawCursor(Int32 x, Int32 y);
	
private:
	Screen *screen;
	Rect bounds;
	Painter *painter;

	LinkedList *window_list;

	Sem list_sem;
};

extern "C" {
#endif

	void _desktop_run(void);

#ifdef __cplusplus
}
#endif

#endif
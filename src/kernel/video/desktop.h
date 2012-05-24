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
	#include "mouse.h"
}

#include "rect.h"

class Window;

class Desktop {
public:

	Desktop(Screen *screen);
	~Desktop(void);

	void AddWindow(Window *window);
	void CloseWindow(Window *window);

	Rect& GetBounds(void) { return bounds; }
	Painter *GetPainter(void) { return painter; }

	void HandleMouseEvent(MouseEvent *event);

	void Draw(void);
	void DrawCursor(void);

	void Invalidate(void) { dirty = true; }
	
private:
	Screen *screen;
	Rect bounds;
	Painter *painter;

	linked_list_t *window_list;
	Window *focused_window;
	Int32 cursor_x, cursor_y;
	bool dirty;

	Sem list_sem;
};

extern "C" {
#endif

	void _desktop_run(void);

#ifdef __cplusplus
}
#endif

#endif

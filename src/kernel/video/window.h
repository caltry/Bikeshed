/*
** File:	window.h
**
** Author:	Sean Congden
**
** Description:	
*/

#ifndef _WINDOW_H
#define _WINDOW_H

#include "uicomponent.h"
#include "painter.h"
#include "desktop.h"

class Window {
public:

	Window(Desktop *desktop, Rect bounds, char *title, int id);
	~Window(void);

	void Move(Uint32 x, Uint32 y);

	Rect& GetBounds(void) { return bounds; }
	bool IsDirty(void) { return dirty; }

	void Repaint(void);

protected:
	void AddComponent(UIComponent *component);

	void Invalidate(void);

private:

	void Draw(void);

	Rect bounds;
	char *title;
	LinkedList *children;
	Painter *painter;

	bool dirty;
	Desktop *desktop;
	int id;
};

#endif

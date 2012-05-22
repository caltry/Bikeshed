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

class Window : public UIComponent {
public:

	Window(Desktop *desktop, Rect bounds, char *title);
	~Window(void);

	void Move(Uint32 x, Uint32 y);

	void SetFocused(bool focus) { this->hasFocus = focus; };

	char *title;

private:
	void Draw(void);
	
	Desktop *desktop;
	bool hasFocus;
};

#endif

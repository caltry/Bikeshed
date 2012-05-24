/*
** File:	window.h
**
** Author:	Sean Congden
**
** Description:	An application window that can be drawn on screen
*/

#ifndef _WINDOW_H
#define _WINDOW_H

#include "uicomponent.h"
#include "painter.h"
#include "desktop.h"


/*
** A window is the basis of a graphical program in bikeshed.
** Graphical applications can extend this class and add their own views
** with the AddComponent method.
*/
class Window : public UIComponent {
public:

	/*
	** Window(desktop,bounds,title)
	**
	** Creates a window that is placed on the provided desktop.
	** The window is positioned and sized based on the bounds parameter
	** and is named with the provided title parameter.
	*/
	Window(Desktop *desktop, Rect bounds, char *title);

	virtual ~Window(void);

	/*
	** Move(x,y)
	**
	** Moves the window to the given x and y coordinates on screen.
	*/
	void Move(Uint32 x, Uint32 y);

	/*
	** SetFocused(focus)
	**
	** Marks the current window as focused which draws the
	** window in a different style.
	*/
	void SetFocused(bool focus) { this->hasFocus = focus; };

private:
	/*
	** Draw()
	**
	** Draws the window decor of the application.
	*/
	void Draw(void);
	
	Desktop *desktop;
	bool hasFocus;
	char *title;
};


#endif

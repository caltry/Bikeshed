/*
** File:	desktop.h
**
** Author:	Sean Congden
**
** Description:	A graphical desktop that can contain graphical applications.
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

	/*
	** Desktop(screen)
	**
	** A graphical desktop tied to the provided screen that can
	** contain graphical applications.
	*/
	Desktop(Screen *screen);
	~Desktop(void);

	/*
	** AddWindow(window)
	**
	** Adds the given window to the desktop.  The window is added
	** to the top of the window stack and gains focus.
	*/
	void AddWindow(Window *window);

	/*
	** CloseWindow(window)
	**
	** Closes and removes the given window from the desktop.
	*/
	void CloseWindow(Window *window);

	/*
	** GetBounds(bounds)
	**
	** Provides a rectangle representing the size of the desktop
	** being rendered.  This is usually the same as the screen resolution.
	** 
	** Returns:
	**      the bounding area of the desktop
	*/
	Rect& GetBounds(void) { return bounds; }

	/*
	** GetPainter()
	**
	** Provides a painter linked to the desktop's screen
	**
	** Returns:
	**      a painter for the desktop
	*/
	Painter *GetPainter(void) { return painter; }

	/*
	** HandleMouseEvent(event)
	**
	** Updates the desktop cursor based on the mouse event then dispatches
	** the event to the focused window.
	*/
	void HandleMouseEvent(MouseEvent *event);

	/*
	** Draw()
	**
	** Paints the entire desktop including all windows it contains.
	*/
	void Draw(void);

	/*
	** DrawCursor()
	**
	** Draws a mouse cursor directly to the screen framebuffer.
	*/
	void DrawCursor(void);

	/*
	** Invalidate()
	**
	** Marks the entire desktop as dirty forcing a redraw of the
	** whole desktop and all of its windows.
	*/
	void Invalidate(void) { dirty = true; }
	
private:
	Screen *screen;
	Rect bounds;
	Painter *painter;

	linked_list_t *window_list;
	Int32 cursor_x, cursor_y;
	bool dirty;
};

extern "C" {
#endif

	/*
	** _desktop_run()
	**
	** Starts up VESA based graphics on the system with a basic set
	** of windows then repaints them in a loop.  This method should
	** be invoked in its own process.
	*/
	void _desktop_run(void);

#ifdef __cplusplus
}
#endif

#endif

/*
** File:	window.cpp
**
** Author:	Sean Congden
**
** Description:	An application window that can be drawn on screen
*/

extern "C" {
	#include "defs.h"
	#include "kmalloc.h"
	#include "linkedlist.h"
	#include "lib/string.h"
	#include "lib/klib.h"
	#include "input/mouse.h"
	#include "serial.h"
}

#include "lookandfeel.h"
#include "rect.h"

#include "window.h"


Window::Window(Desktop *desktop, Rect bounds, char *title)
	: UIComponent(desktop->GetPainter(), bounds)
	, desktop(desktop)
	, has_focus(false)
	, close_bounds(Rect(bounds.width - 16, 8, 16, 16))
{
	// Probably a good idea to save a copy, in case the string was
	// created in a temporary location, ran into this problem with
	// the ELF loader
	this->title = (char *)__kmalloc(_kstrlen(title) + 1);
	_kmemcpy(this->title, title, _kstrlen(title) + 1);
}


Window::~Window(void)
{
}


void Window::Move(Int32 x, Int32 y)
{
	//TODO: lock desktop wite lock

	UIComponent::Move(x, y);

	desktop->Invalidate();

	//TODO: unlock desktop write lock
}


void Window::Draw(void)
{
	Uint32 focusColor = (has_focus) ? WINDOW_ACTIVE_COLOR
		 : WINDOW_INACTIVE_COLOR;
	
	// Draw the window base
	painter->DrawBox(bounds, WINDOW_BASE_COLOR_1, WINDOW_BASE_COLOR_2,
		WINDOW_BASE_COLOR_3, WINDOW_BASE_COLOR_4, 4);

	// Draw the title bar
	int ysize = 32;
	Rect titlebar = Rect(bounds.x, bounds.y, bounds.width, ysize);
	painter->DrawBox(titlebar, WINDOW_TITLE_COLOR_1, WINDOW_TITLE_COLOR_2,
		WINDOW_TITLE_COLOR_3, WINDOW_TITLE_COLOR_4, 4);

	// Draw the title text
	painter->DrawString(title, bounds.x + 12, bounds.y + 2, 4, focusColor);

	// Draw the close button
	Rect close = Rect(bounds.x2 - WINDOW_CLOSE_OFFSET - WINDOW_CLOSE_X_SIZE,
		bounds.y + WINDOW_CLOSE_Y_SIZE + 10, WINDOW_CLOSE_X_SIZE,
		WINDOW_CLOSE_Y_SIZE);
	painter->FillRect(close, focusColor);
}


void Window::HandleMouseEvent(MouseEvent *event)
{
	switch (event->type) {
		case MOUSE_EVENT_CLICKED: {
			// Check if the close button is clicked
			if (close_bounds.Contains(event->x, event->y)) {
				serial_string("close window\n");
				desktop->CloseWindow(this);
			}
			break;
		}

		case MOUSE_EVENT_DRAGGED: {
			// Move the window if the title bar is dragged
			if (event->start_y < 32) {
				Move(bounds.x + event->delta_x, bounds.y - event->delta_y);
			}

			break;
		}
	}

	event->consumed = 1;
}

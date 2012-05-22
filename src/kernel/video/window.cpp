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
}

#include "lookandfeel.h"
#include "rect.h"

#include "window.h"


Window::Window(Desktop *desktop, Rect bounds, char *title)
	: UIComponent(desktop->GetPainter(), bounds)
	, desktop(desktop)
	, hasFocus(false)
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


void Window::Move(Uint32 x, Uint32 y)
{
	//TODO: lock desktop wite lock

	bounds.SetPosition(x, y);
	Rect* rect = desktop->GetBounds().Intersection(bounds);
	painter->SetClipping(*rect);
	delete rect;

	//TODO: unlock desktop write lock
}


void Window::Draw(void)
{
	Uint32 focusColor = (hasFocus) ? WINDOW_ACTIVE_COLOR
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

	// Draw the close buttor
	int close_x = 16;
	int close_y = 4;
	int closeoff = (ysize - close_y) / 2;
	Rect close = Rect(bounds.x2 - closeoff - close_x, bounds.y + closeoff,
		close_x, close_y);
	painter->FillRect(close, focusColor);
}

/*
** File:	window.cpp
**
** Author:	Sean Congden
**
** Description:	A basic rectangle
*/

extern "C" {
	#include "defs.h"
	#include "kmalloc.h"
	#include "linkedlist.h"
}

#include "lookandfeel.h"
#include "rect.h"

#include "window.h"


Window::Window(Desktop *_desktop, Rect _bounds, char *_title, int _id)
	: bounds(_bounds)
	, title(_title)
	, dirty(true)
	, desktop(_desktop)
	, id(_id)
{
	children = (LinkedList *)__kmalloc(sizeof(LinkedList));
	list_init(children, __kfree);
	painter = new Painter(kScreen, bounds);
}


Window::~Window(void)
{
	list_destroy(children);
	__kfree(children);
	delete painter;
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


void Window::AddComponent(UIComponent *component)
{
	list_insert_next(children, NULL, component);
}


void Window::Invalidate(void)
{
	dirty = true;
}


void Window::Repaint(void)
{
	if (!dirty) return;
	Draw();

	ListElement *cur_node = list_head(children);
	while (cur_node != NULL) {
		//((UIComponent *)list_data(cur_node))->Repaint();

		cur_node = cur_node->next;
	}

	//dirty = false;
}


void Window::Draw(void)
{
	// Draw the window base
	Uint32 color = WINDOW_BASE_COLOR_1;
	if (id != 0) {
		if (id == 2) {
			color = WINDOW_TITLE_COLOR_1;	
			id = 1;
		} else if (id == 1) {
			id = 2;
		}

		painter->DrawBox(bounds, color, WINDOW_BASE_COLOR_2,
			WINDOW_BASE_COLOR_3, WINDOW_BASE_COLOR_4, 4);

		dirty = true;
	} else {
		painter->DrawBox(bounds, WINDOW_TITLE_COLOR_1, WINDOW_TITLE_COLOR_2,
			WINDOW_TITLE_COLOR_3, WINDOW_TITLE_COLOR_4, 4);
		dirty = false;
	}

	// Draw the title bar
	int ysize = 32;
	Rect titlebar = Rect(bounds.x, bounds.y, bounds.width, ysize);
	painter->DrawBox(titlebar, WINDOW_TITLE_COLOR_1, WINDOW_TITLE_COLOR_2,
		WINDOW_TITLE_COLOR_3, WINDOW_TITLE_COLOR_4, 4);

	// Draw the close buttor
	int close_x = 32;
	int close_y = 16;
	int closeoff = (ysize - close_y) / 2;
	Rect close = Rect(bounds.x2 - closeoff - close_x, bounds.y + closeoff,
		close_x, close_y);
	painter->DrawBox(close, WINDOW_TITLE_COLOR_1, WINDOW_TITLE_COLOR_2,
		WINDOW_TITLE_COLOR_2, WINDOW_TITLE_COLOR_2, 4);
}






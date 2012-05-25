/*
** File:	desktop.cpp
**
** Author:	Sean Congden
**
** Description:	A graphical desktop that can contain graphical applications.
*/

extern "C" {
	#include "defs.h"
	#include "kmalloc.h"
	#include "klib.h"
	#include "ulib.h"
	#include "linkedlist.h"
	#include "video.h"
	#include "events.h"
	#include "mouse.h"
	#include "semaphores.h"
	#include "serial.h"
}

#include "window.h"
#include "painter.h"
#include "painter24.h"
#include "painter32.h"
#include "rect.h"
#include "region.h"
#include "gconsole.h"
#include "lookandfeel.h"

#include "desktop.h"

static
void delete_window(void* data)
{
	Window* window = (Window *)data;
	delete window;
}

Desktop::Desktop(Screen *_screen)
	: screen(_screen)
	, bounds(Rect(0, 0, _screen->width, _screen->height))
	, cursor_x(_screen->width / 2)
	, cursor_y(_screen->height /2)
	, dirty(true)
{
	window_list = (linked_list_t *)__kmalloc(sizeof(linked_list_t));
	list_init(window_list, delete_window);

	// Create the graphics painters
	if (screen->bpp == 24) {
		painter = new Painter24(screen, bounds);
	} else if (screen->bpp == 32) {
		painter = new Painter32(screen, bounds);
	}
}


Desktop::~Desktop(void)
{
	list_destroy(window_list);
	__kfree(window_list);
	delete painter;
}


void Desktop::AddWindow(Window *window)
{
	ListElement *focused_elem = list_tail(window_list);
	if (focused_elem != NULL) {
		((Window *)list_data(focused_elem))->SetFocused(false);
	}
		
	list_insert_next(window_list, focused_elem, (void *)window);

	window->SetFocused(true);
	dirty = true;
}

void Desktop::CloseWindow(Window *window)
{
	ListElement *element = list_head(window_list);

	while (element != NULL) {
		if ((Window *)list_data(element) == window) {
			list_remove_next(window_list, element, NULL);
			break;
		}

		element = list_next(element);
	}
	
	dirty = true;
}

void Desktop::Draw(void)
{
	// Draw all of the windows
	ListElement *cur_node = list_head(window_list);
	//Region *updateRegion = new Region();

	if (dirty) {
		// Redraw the whole background
		painter->Fill(DESKTOP_BACKGROUND_COLOR);
	}

	while (cur_node != NULL) {
		Window *window = (Window *)list_data(cur_node);

		// Only draw a window if it is dirty or a window it
		//   is overlapping has been repainted
		//if (dirty || window->IsDirty() ||
		//	updateRegion->Intersects(window->GetBounds()))
		//{
			window->Repaint();
		//	updateRegion->AddRect(window->GetBounds());
		//}

		cur_node = list_next(cur_node);
	}

	//delete updateRegion;
	dirty = false;
}

#define CLAMP(x, min, max)	((x < min) ? min : (x > max) ? max : x)

void Desktop::HandleMouseEvent(MouseEvent *event)
{
	// Apply the mouse coordinate changes
	Int32 start_x = cursor_x;
	Int32 start_y = cursor_y;
	cursor_x = CLAMP(cursor_x + event->delta_x, 0, (Int32)bounds.width);
	cursor_y = CLAMP(cursor_y - event->delta_y, 0, (Int32)bounds.height);

	// Draw all of the windows
	ListElement *cur_node = list_tail(window_list);
	while (cur_node != NULL) {
		Window *window = (Window *)list_data(cur_node);

		// Offset the x and y coordinates to window space
		Rect win_bounds = window->GetBounds();
		event->start_x = start_x - win_bounds.x;
		event->start_y = start_y - win_bounds.y;
		event->x = cursor_x - win_bounds.x;
		event->y = cursor_y - win_bounds.y;

		// Pass the event to a window based on the starting x and y coords.
		if ((start_x >= win_bounds.x && start_x <= win_bounds.x2) &&
			(start_y >= win_bounds.y && start_y <= win_bounds.y2))
		{
			window->HandleMouseEvent(event);
			if (event->consumed)
				return;
		}

		cur_node = list_prev(cur_node);
	}
}

void Desktop::DrawCursor(void)
{
	painter->DrawCursor(cursor_x, cursor_y);
}


extern "C" {
	void _desktop_run(void) {
		asm volatile("cli");

		/*
		** Start up the system graphics module
		*/
		serial_printf("Enabling video...\n");
		_video_init();
		serial_printf("Enabling mouse...\n");
		_events_init();
		_mouse_init();

		// Create a desktop
		Desktop desktop(kScreen);

		// Add the graphical console window
		_gconsole = new GConsole(&desktop, 16, 16);

		// Add some initial windows to the desktop
		Window *window = new Window(&desktop,
			Rect(200, 200, 400, 400), (char *)"BIKESHED");

		desktop.AddWindow(_gconsole);
		desktop.AddWindow(window);

		Uint32 event_type;
		void *event;

		serial_printf("Entering draw loop...\n");
		do {
			// Check for new event messages
			while (_events_fetch(&event, &event_type) != EMPTY_QUEUE) {
				// Handle the event
				if (event_type == EVENT_MOUSE) {
					desktop.HandleMouseEvent((MouseEvent *)event);
				}

				// Dispose of the event message
				asm volatile("cli");
				__kfree(event);
				asm volatile("sti");
			}

			// Repaint the desktop
			desktop.Draw();

			asm volatile("cli");
			// Copy the back buffer to the screen
			_kmemcpy((void *)(kScreen->frame_buffer),
				(void *)(kScreen->back_buffer), kScreen->size);

			// Draw the mouse
			desktop.DrawCursor();
			asm volatile("sti");

#ifdef QEMU
			// Update at about 100 fps
			msleep(20);
#endif
		} while ( 1 );
	}
}

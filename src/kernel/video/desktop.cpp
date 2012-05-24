/*
** File:	desktop.cpp
**
** Author:	Sean Congden
**
** Description:	
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
{
	window_list = (linked_list_t *)__kmalloc(sizeof(linked_list_t));
	list_init(window_list, delete_window);

	//sem_init(&list_sem);
	//sem_post(list_sem);

	// Create the graphics painters
	if (screen->bpp == 24) {
		painter = new Painter24(screen, bounds);
	} else if (screen->bpp == 32) {
		painter = new Painter32(screen, bounds);
	}

	// Draw the background
	painter->Fill(0x6490a7);
}


Desktop::~Desktop(void)
{
	list_destroy(window_list);
	__kfree(window_list);
	delete painter;
}


void Desktop::AddWindow(Window *window)
{
//	sem_wait(list_sem);
	list_insert_next(window_list, NULL, (void *)window);
//	sem_post(list_sem);
}

void Desktop::Draw(void)
{
//	sem_wait(list_sem);

	// Draw all of the windows
	ListElement *cur_node = list_head(window_list);
	Region *updateRegion = new Region();

	while (cur_node != NULL) {
		Window *window = (Window *)list_data(cur_node);

		// Only draw a window if it is dirty or a window it
		//   is overlapping has been repainted
		if (window->IsDirty() ||
			updateRegion->Intersects(window->GetBounds()))
		{
			window->Repaint();
			updateRegion->AddRect(window->GetBounds());
		}

		cur_node = list_next(cur_node);
	}

	delete updateRegion;

//	sem_post(list_sem);
}

#define CLAMP(x, min, max)	((x < min) ? min : (x > max) ? max : x)

void Desktop::HandleMouseEvent(MouseEvent *event)
{
	// Apply the mouse coordinate changes
	cursor_x = CLAMP(cursor_x + (event->delta_x * 2), 0, (Int32)bounds.width);
	cursor_y = CLAMP(cursor_y - (event->delta_y * 2), 0, (Int32)bounds.height);

	//serial_printf("Mouse:  x: %d  y:  %d  states:  %d  active: %d type: %d\n",
	//	event->delta_x, event->delta_y, event->button_states,
	//	event->active_button, event->type);
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
		gcon_init();
		_gconsole = new GConsole(&desktop, 16, 16);

		// Add some initial windows to the desktop
		Window *window = new Window(&desktop,
			Rect(200, 200, 400, 400), (char *)"BIKESHED");
		window->SetFocused(true);

		desktop.AddWindow(window);
		desktop.AddWindow(_gconsole);

		Uint32 event_type;
		void *event;

		serial_printf("Entering draw loop...\n");
		do {
			asm volatile("cli");
			
			// Check for new event messages
			while (_events_fetch(&event, &event_type) != EMPTY_QUEUE) {
				// Handle the event
				if (event_type == EVENT_MOUSE) {
					desktop.HandleMouseEvent((MouseEvent *)event);
				}

				// Dispose of the event message
				__kfree(event);
			}

			// Repaint the desktop
			desktop.Draw();

			// Copy the back buffer to the screen
			_kmemcpy((void *)(kScreen->frame_buffer),
				(void *)(kScreen->back_buffer), kScreen->size);

			// Draw the mouse
			desktop.DrawCursor();

			// Update at about 200 fps
			asm volatile("sti");

			msleep(10);
		} while ( 1 );
	}
}

/*
** File:	desktop.cpp
**
** Author:	Sean Congden
**
** Description:	
*/

extern "C" {
	#include "kmalloc.h"
	#include "klib.h"
	#include "ulib.h"
	#include "linkedlist.h"
	#include "video.h"
	#include "mouse.h"
}

#include "window.h"
#include "painter.h"
#include "painter24.h"
#include "painter32.h"
#include "rect.h"
#include "region.h"
#include "gconsole.h"

#include "desktop.h"


Desktop::Desktop(Screen *screen)
	: screen(screen)
	, bounds(Rect(0, 0, screen->width, screen->height))
{
	window_list = (LinkedList *)__kmalloc(sizeof(window_list));
	list_init(window_list, __kfree);

	sem_init(&list_sem);
	sem_post(list_sem);

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
}


void Desktop::AddWindow(Window *window)
{
	sem_wait(list_sem);
	list_insert_next(window_list, NULL, (void *)window);
	sem_post(list_sem);
}


void Desktop::Draw(void)
{
	sem_wait(list_sem);

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

	sem_post(list_sem);
}


void Desktop::DrawCursor(Int32 x, Int32 y)
{
	painter->DrawCursor(x, y);
}


extern "C" {
	void _desktop_run(void) {

		/*
		** Start up the system graphics module
		*/
		_video_init();
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

		do {
			// Repaint the desktop
			desktop.Draw();

			// Copy the back buffer to the screen
			_kmemcpy((void *)(kScreen->frame_buffer),
				(void *)(kScreen->back_buffer), kScreen->size);

			// Draw the mouse
			desktop.DrawCursor(_mouse_x, _mouse_y);
		
			// Update at about 200 fps
			msleep(10);
		} while ( 1 );
	}
}
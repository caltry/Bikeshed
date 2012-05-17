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
}

#include "window.h"
#include "painter.h"
#include "rect.h"
#include "region.h"

#include "desktop.h"


Desktop::Desktop(Screen *screen)
	: screen(screen)
	, bounds(Rect(0, 0, screen->width, screen->height))
{
	window_list = (LinkedList *)__kmalloc(sizeof(window_list));
	list_init(window_list, __kfree);

	sem_init(&list_sem);
	sem_post(list_sem);
	painter = new Painter(screen, bounds);
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

	ListElement *cur_node = list_head(window_list);
	Region *updateRegion = new Region();

	while (cur_node != NULL) {
		Window *window = (Window *)list_data(cur_node);

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


extern "C" {
	void _desktop_run(void) {
		/*
		** Start up the system graphics module
		*/
		_video_init();

		Desktop desktop(kScreen);

		Window *window = new Window(&desktop, Rect(200, 200, 400, 400), "BIKESHED", 1);
		Window *window2 = new Window(&desktop, Rect(16, 16, 300, 400), "BIKESHED", 0);
		desktop.AddWindow(window);
		desktop.AddWindow(window2);

		while( 1 ) {
			//gconsole_draw(16, 16);
			desktop.Draw();

			//sem_wait(kScreen->buffer_lock);

			// Copy the back buffer to the screen
			_kmemcpy((void *)(kScreen->frame_buffer),
				(void *)(kScreen->back_buffer), kScreen->size);
			//c_puts("test\n");

			//sem_post(kScreen->buffer_lock);

			// Update at about 200 fps
			msleep(5);
		}
	}
}
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
	#include "semaphores.h"
	#include "serial.h"
}

#include "window.h"
#include "painter.h"
#include "rect.h"
#include "region.h"

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
{
	window_list = (linked_list_t *)__kmalloc(sizeof(linked_list_t));
	list_init(window_list, delete_window);

	//sem_init(&list_sem);
	//sem_post(list_sem);
	painter = new Painter(screen, bounds);
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

	ListElement *cur_node = list_head(window_list);
	Region *updateRegion = new Region();

	while (cur_node != NULL) {
		serial_printf("Top loop\n");
		serial_printf("Drawing window.. %x", cur_node);
		Window *window = (Window *)list_data(cur_node);

		serial_printf("Checking if dirty\n");
		if (window->IsDirty() ||
			updateRegion->Intersects(window->GetBounds()))
		{
			serial_printf("repainting\n");
			window->Repaint();
			serial_printf("adding rect\n");
			updateRegion->AddRect(window->GetBounds());
			serial_printf("Done adding rectangle\n");
		}

		serial_printf("Getting next\n");
		cur_node = list_next(cur_node);
		serial_printf("Done getting next\n");
	}

	serial_printf("Deleting region...\n");
	delete updateRegion;

//	sem_post(list_sem);

	serial_printf("Done posting\n");
}

extern "C" {
	void _desktop_run(void) {
		asm volatile("cli");
		/*
		** Start up the system graphics module
		*/
		_video_init();

		const Uint16 * const orig_frame_buffer = kScreen->frame_buffer;
		const Uint16 * const  orig_back_buffer  = kScreen->back_buffer;
		const Uint32 size = kScreen->size;

		Desktop desktop(kScreen);

		// Created desktop
		serial_printf("===================================CREATED DESKTOP=========================================\n");

		serial_printf("Frame buffer location: %x\n", orig_frame_buffer);
		serial_printf("Back buffer location:  %x\n", orig_back_buffer);
		serial_printf("Max location: %x\n", (Uint32)orig_back_buffer + kScreen->size);

		Window *window = new Window(&desktop, Rect(200, 200, 400, 400), "BIKESHED", 1);
		Window *window2 = new Window(&desktop, Rect(16, 16, 300, 400), "BIKESHED", 0);
		desktop.AddWindow(window);
		desktop.AddWindow(window2);

		while( 1 ) {
			asm volatile("cli");
			//gconsole_draw(16, 16);
			desktop.Draw();

			if (kScreen->frame_buffer != orig_frame_buffer ||
					kScreen->back_buffer != orig_back_buffer) {
				_kpanic("Desktop", "Frambuffer pointers changed!\n", (Status)0);
			}
			//sem_wait(kScreen->buffer_lock);

			serial_printf("Swapping buffers\n");
			// Copy the back buffer to the screen
			_kmemcpy((void *)(kScreen->frame_buffer),
				(void *)(kScreen->back_buffer), kScreen->size);
			//c_puts("test\n");
			serial_printf("About to sleep\n");
			//sem_post(kScreen->buffer_lock);

			// Update at about 200 fps
			asm volatile("sti");
			msleep(5);
			serial_printf(".");
		}
	}
}

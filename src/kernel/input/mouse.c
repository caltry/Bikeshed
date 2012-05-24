/*
** File:	mouse.c
**
** Author:	Sean Congden
**
** Description:	
*/

#include "support.h"
#include "startup.h"
#include "x86arch.h"
#include "memory/kmalloc.h"
#include "events.h"

#include "mouse.h"


static Uint8 last_button_states;


static void _mouse_wait(Uint32 type)
{
	Uint32 timeout = 100000;

	if (type == 0) {	// Data
		while (timeout--) {
			if ((__inb(0x64) & 1) == 1)
				return;
		}
	} else {			// Signal
		while (timeout--) {
			if ((__inb(0x64) & 2) == 0)
				return;
		}
	}
}


static void _mouse_write(Uint8 byte)
{
	// Tell the mouse we are sending a command
	_mouse_wait(1);
	__outb(0x64, 0xd4);

	// Wait for some reason...
	_mouse_wait(1);

	// Send the command
	__outb(0x60, byte);
}


static Uint8 _mouse_read(void)
{
	// Wait for the mouse
	_mouse_wait(0);

	// Get the response
	return __inb(0x60);
}


Status _mouse_init(void)
{
	// Buttons are initially unpressed
	last_button_states = 0;

	asm volatile("cli");

	// Send the initial enable message
	_mouse_wait(1);
	__outb(0x64, 0xa8);

	// Request the status byte
	_mouse_wait(1);
	__outb(0x64, 0x20);

	// Retreive the status byte
	_mouse_wait(0);
	Uint8 status = __inb(0x60);

	// Modify the status to enable interrupts
	status |= 2;		// Set the interrupt enable bit
	status &= 0xdf;		// Turn off the mouse disable bit

	// Tell the mouse we are going to send the new status
	_mouse_wait(1);
	__outb(0x64, 0x60);

	// Send the status byte
	_mouse_wait(1);
	__outb(0x60, status);

	// Tell the mouse to use its default settings and wait for a response
	_mouse_write(0xf6);
	_mouse_read();

	// Enable the mouse and wait for the response
	_mouse_write(0xf4);
	_mouse_read();

	// Install the mouse isr
	__install_isr(0x2c, _mouse_isr);

	asm volatile("sti");

	return SUCCESS;
}


#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
void _mouse_isr(int vector, int code)
{
#ifdef QEMU
	static Int8 byte_cycle = -1;
#else
	static Int8 byte_cycle = 0;
#endif

	static Int8 bytes[3];

	bytes[byte_cycle++] = __inb(0x60);

	if (byte_cycle == 3) {
		// Reset the byte cycle count
		byte_cycle = 0;

		// Create a new mouse event
		MouseEvent *event = (MouseEvent *)__kmalloc(sizeof(MouseEvent));

		// Initialize the event
		event->button_states = bytes[0] & 0x7;
		event->start_x = 0;
		event->start_y = 0;
		event->x = 0;
		event->y = 0;
		event->delta_x = bytes[1];
		event->delta_y = bytes[2];
		event->active_button = 0;
		event->type = MOUSE_EVENT_MOVED;
		event->consumed = 0;

		// Update the x and y coordinate threshold

		// Check which buttons have changed
		if ((bytes[0] & MOUSE_BUTTON_LEFT) !=
			(last_button_states & MOUSE_BUTTON_LEFT))
		{
			event->active_button = MOUSE_BUTTON_LEFT;
		} else if ((bytes[0] & MOUSE_BUTTON_RIGHT) !=
			(last_button_states & MOUSE_BUTTON_RIGHT))
		{
			event->active_button = MOUSE_BUTTON_RIGHT;
		} else if ((bytes[0] & MOUSE_BUTTON_MIDDLE) !=
			(last_button_states & MOUSE_BUTTON_MIDDLE))
		{
			event->active_button = MOUSE_BUTTON_MIDDLE;
		}

		if (event->active_button) {
			// If any buttons have changed, determine the type of mouse event 
			event->type = (bytes[0] & event->active_button)
				? MOUSE_EVENT_PRESSED : MOUSE_EVENT_CLICKED;
		} else if (bytes[0] & 0x7) {
			// If there are no new button changes and a button is pressed
			//   then the mosue is being dragged
			event->type = MOUSE_EVENT_DRAGGED;
		}

		// Send the event
		_events_dispatch((void *)event, EVENT_MOUSE);
		
		// Save the button states
		last_button_states = bytes[0];
	}

	// Signal the end of the interrupt
	__outb( PIC_MASTER_CMD_PORT, PIC_EOI );
	__outb( PIC_SLAVE_CMD_PORT, PIC_EOI );
}
#pragma GCC diagnostic pop

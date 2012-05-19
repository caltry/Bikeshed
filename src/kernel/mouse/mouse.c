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
#include "video.h"
#include "gconsole.h"

#include "mouse.h"


Int32 _mouse_x, _mouse_y;
Int32 x_vel, y_vel;

int left_pressed, right_pressed, middle_pressed;


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
	// Set the initial mouse coordinates to the center of the screen
	_mouse_x = kScreen->width / 2;
	_mouse_y = kScreen->height / 2;

	// Buttons are initially unpressed
	left_pressed = 0;
	right_pressed = 0;

	// Disable the keyboard for a bit
	//_mouse_wait(1);
	//__outb(0x64, 0xad);

	// Send the initial enable message
	_mouse_wait(1);
	__outb(0x64, 0xa8);

	// Reset the mouse
	//_mouse_write(0xff);

	// Wait for reset completion
	/*_mouse_wait(0);
	__inb(0x60);					// Reset ACK
	if (__inb(0x60) != 0xaa)		// Check reset success
		return BAD_PARAM;*/

	// Get the mouse id
	//__inb(0x60);

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

	return SUCCESS;
}


#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
void _mouse_isr(int vector, int code)
{
	static Int8 byte_cycle = -1;
	static Int8 bytes[3];

	bytes[byte_cycle++] = __inb(0x60);

	if (byte_cycle == 3) {
		// Reset the byte cycle count
		byte_cycle = 0;

		// Get the x and y deltas
		x_vel = bytes[1] * 2;
		y_vel = bytes[2] * 2;

		// Update the x and y positions
		_mouse_x += x_vel;
		_mouse_y -= y_vel;

		if (_mouse_x > kScreen->width)
			_mouse_x = kScreen->width;
		if (_mouse_y > kScreen->height)
			_mouse_y = kScreen->height;
		if (_mouse_x < 0)
			_mouse_x = 0;
		if (_mouse_y < 0)
			_mouse_y = 0;

		// Check the button states
		if (bytes[0] & 0x1) {
			if (!left_pressed) {
				// Left button pressed mouse event
				left_pressed = 1;
			}
		} else {
			left_pressed = 0;
		}

		if (bytes[0] & 0x2) {
			if (!right_pressed) {
				// Right button pressed mouse event
				right_pressed = 1;
			}
		} else {
			right_pressed = 0;
		}

		if (bytes[0] & 0x4) {
			if (!middle_pressed) {
				// Middle button pressed mouse event
				middle_pressed = 1;
			}
		} else {
			middle_pressed = 0;
		}

	}

	// Signal the end of the interrupt
	__outb( PIC_MASTER_CMD_PORT, PIC_EOI );
	__outb( PIC_SLAVE_CMD_PORT, PIC_EOI );
}
#pragma GCC diagnostic pop

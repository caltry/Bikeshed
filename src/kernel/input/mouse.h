/*
** File:	mouse.h
**
** Author:	Sean Congden
**
** Description:	A PS/2 mouse driver
*/

#ifndef _MOUSE_H
#define _MOUSE_H


#include "types.h"


/*
** General definitions
*/

/* Mouse buttons */
#define MOUSE_BUTTON_LEFT				0x1
#define MOUSE_BUTTON_RIGHT				0x2
#define MOUSE_BUTTON_MIDDLE				0x4

/* The types of generated mouse events */
#define MOUSE_EVENT_MOVED				1
#define MOUSE_EVENT_PRESSED				2
#define MOUSE_EVENT_RELEASED			3
#define MOUSE_EVENT_CLICKED				4
#define MOUSE_EVENT_DRAGGED				5


/*
** Types
*/

typedef struct {
	Int32 start_x;
	Int32 start_y;
	Int32 x;
	Int32 y;
	Int32 delta_x;
	Int32 delta_y;
	Uint8 button_states;
	Uint8 active_button;
	Uint8 type;
} MouseEvent;


/*
** Prototypes
*/

Status _mouse_init(void);

void _mouse_isr(int vector, int code);

#endif

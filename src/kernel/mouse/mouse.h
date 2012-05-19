/*
** File:	mouse.h
**
** Author:	Sean Congden
**
** Description:	
*/

#ifndef _MOUSE_H
#define _MOUSE_H

/*
** General definitions
*/

#include "headers.h"
#include "ulib.h"


extern Int32 _mouse_x;
extern Int32 _mouse_y;


/*
** Prototypes
*/

Status _mouse_init(void);

void _mouse_isr(int vector, int code);

#endif

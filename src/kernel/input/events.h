/*
** File:	events.h
**
** Author:	Sean Congden
**
** Description:	A module for dispatching input events to user processes.
** Currently used by the keyboard and mouse drivers.
*/

#ifndef _EVENTS_H
#define _EVENTS_H


#include "types.h"
#include "queues.h"


/*
** General definitions
*/

/* The different types of generated events */
#define EVENT_MOUSE						0x1
#define EVENT_KEYBOARD					0x2


/*
** Prototypes
*/

Status _events_init(void);

Status _events_dispatch(void *event, Uint32 event_type);

Status _events_fetch(void **event, Uint32 *event_type);


#endif

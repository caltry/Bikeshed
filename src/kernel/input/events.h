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
#define EVENT_MOUSE						1
#define EVENT_KEYBOARD					2


/*
** Prototypes
*/

/*
** _events_init()
**
** Initializes the event queue module.  Events are added to
** the queue and pulled off in chronological order.
*/
Status _events_init(void);

/*
** _events_dispatch(event,event_type)
**
** Adds an event to the event queue.
*/
Status _events_dispatch(void *event, Uint32 event_type);

/*
** _events_fetch(event,event_type)
**
** Grabs the oldest input event from the event queue.
** The event_type indicates the type of data the event holds.
*/
Status _events_fetch(void **event, Uint32 *event_type);


#endif

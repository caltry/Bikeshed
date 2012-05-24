/*
** File:	events.c
**
** Author:	Sean Congden
**
** Description:	
*/

#include "defs.h"
#include "queues.h"

#include "events.h"


static Queue *event_queue;


Status _events_init(void) {
	return _q_alloc(&event_queue, NULL);
}


Status _events_dispatch(void *event, Uint32 event_type) {
	Key event_key;
	event_key.u = event_type;
	 
	return _q_insert(event_queue, event, event_key);
}


Status _events_fetch(void **event, Uint32 *event_type) {
	Key event_key;
	
	asm volatile("cli");
	Status status = _q_peek(event_queue, &event_key);
	*event_type = event_key.u;

	if (status == SUCCESS) {
		status = _q_remove(event_queue, event);
		asm volatile("sti");
	}

	return status;
}
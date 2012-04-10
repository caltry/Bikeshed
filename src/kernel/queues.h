/*
** SCCS ID:	@(#)queues.h	1.1	4/5/12
**
** File:	queues.h
**
** Author:	4003-506 class of 20113
**
** Contributor:
**
** Description:	Queue module definitions
*/

#ifndef _QUEUES_H
#define _QUEUES_H

#include "headers.h"

/*
** General (C and/or assembly) definitions
*/

#ifndef __ASM__20113__

/*
** Start of C-only definitions
*/

// Pseudo-function:  is a queue empty?

#define _q_empty(n) (_q_status(n) == EMPTY_QUEUE)

/*
** Types
**
** Our queues are generic, self-ordering queues.  The queue itself
** is a doubly-linked list of queue nodes, which in turn point to
** the information queued up in this position.  Each queue has an
** associated ordering routine which is used by the insertion 
** routine to order the queue elements.  This allows us to have
** different sorting criteria for different queues, but manage them
** with one set of functions.
**
** From the outside world, "Queue" is an opaque type.  However, inside
** the queue module itself, we need the real Queue declaration.  We
** control this with the COMPILING_QUEUE_MODULE symbol.
*/

#ifndef COMPILING_QUEUE_MODULE

// Queue type forward declaration

struct queue;
typedef struct queue Queue;

#endif

/*
** Globals
*/

/*
** Prototypes
*/

/*
** _comp_ascend_int(key1,key2)
**
** compare the two keys as signed integer values
**
** returns the status of the comparison:
**	< 0	key1 < key2
**	= 0	key1 == key2
**	> 0	key1 > key2
*/

int _comp_ascend_int( Key, Key );

/*
** _comp_ascend_uint(key1,key2)
**
** compare the two keys as unsigned integer values
**
** returns the status of the comparison:
**	< 0	key1 < key2
**	= 0	key1 == key2
**	> 0	key1 > key2
*/

int _comp_ascend_uint( Key, Key );

/*
** _q_init( void )
**
** initialize the queue module
*/

void _q_init( void );

/*
** _q_status(queue)
**
** evaluate the status of a queue
**
** returns the status
*/

Status _q_status( Queue * );

/*
** _q_alloc(queue,compare)
**
** allocate a queue which uses the specified comparison routine,
** returning a pointer to the queue through the first parameter
**
** returns the status of the allocation attempt
*/

Status _q_alloc( Queue **, int (*compare)(Key,Key) );

/*
** _q_peek(queue,key)
**
** peek into the specified queue, returning the first Key through
** the second parameter
**
** returns the status of the peek attempt
*/

Status _q_peek( Queue *, Key * );

/*
** _q_insert(queue,data,key)
**
** insert the supplied data value into a queue, using the supplied
** key for ordering according to the queue's ordering rule
**
** returns the status of the insertion attempt
*/

Status _q_insert( Queue *, void *, Key );

/*
** _q_remove(queue,data)
**
** remove the first element from the queue, returning the pointer to
** it through the second parameter
**
** returns the status of the removal attempt
*/

Status _q_remove( Queue *, void ** );

/*
** _q_remove_by_key(queue,data,key)
**
** remove the first element in the queue which has the supplied key,
** returning the pointer to it through the second parameter
**
** returns the status of the removal attempt
*/

Status _q_remove_by_key( Queue *, void **, Key );

/*
** _q_remove_selected(queue,data,compare,lookfor)
**
** scan a queue looking for a particular entry (specified by the lookfor
** parameter) using the supplied comparison function, removing the node,
** and returning the pointer to the node through the second parameter
**
** returns the status of the removal attempt
*/

Status _q_remove_selected( Queue *, void **,
				int (*compare)(void *, void *),
				void *lookfor );

/*
** _q_dump(which,queue)
**
** dump the contents of the specified queue to the console
*/

void _q_dump( char *which, Queue *queue );

#endif

#endif

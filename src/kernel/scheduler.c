/*
** SCCS ID:	@(#)scheduler.c	1.1	4/5/12
**
** File:	scheduler.c
**
** Author:	4003-506 class of 20113
**
** Contributor:
**
** Description:	Scheduler module
*/

#define	__KERNEL__20113__

#include "headers.h"

#include "scheduler.h"

/*
** PRIVATE DEFINITIONS
*/

/*
** PRIVATE DATA TYPES
*/

/*
** PRIVATE GLOBAL VARIABLES
*/

/*
** PUBLIC GLOBAL VARIABLES
*/

Pcb *_current;		// the current process

// the array of ready queues for our MLQ structure
// must be public so that the clock ISR can check it

Queue *_ready[ N_READYQ ];

/*
** PRIVATE FUNCTIONS
*/

/*
** PUBLIC FUNCTIONS
*/

/*
** _sched_init()
**
** initialize the scheduler module
*/

void _sched_init( void ) {
	int i;
	Status status;

	for( i = 0; i < N_READYQ; ++i ) {
		status = _q_alloc( &_ready[i], NULL );
		if( status != SUCCESS ) {
			_kpanic( "_sched_init",
				 "readyq alloc status %s",
				 status );
		}
	}

	_current = NULL;

	c_puts( " scheduler" );
}


/*
** _sched(pcb)
**
** schedule a process for execution according to is priority
**
** returns the status of the insertion into the ready queue
*/

Status _sched( Pcb *pcb ) {
	Prio p;
	Key key;

	if( pcb == NULL ) {
		return( BAD_PARAM );
	}

	// Insert into one of the N ready queues
	// according to the priority of the process

	p = pcb->priority;
	if( p >= N_READYQ ) {
		return( BAD_PRIO );
	}

	key.u = pcb->pid;
	pcb->state = READY;
	return( _q_insert( _ready[p], (void *) pcb, key ) );

}


/*
** _dispatch()
**
** give the CPU to a process
*/

void _dispatch( void ) {
	int i;
	Status status;

	// select a process from the highest-priority
	// ready queue that is not empty

	for( i = 0; i < N_READYQ; ++i ) {

	   do {

		if( _q_empty(_ready[i]) ) {
			break;
		}

		// found one - make it the currently-running process
		status = _q_remove( _ready[i], (void **) &_current );
		if( status == SUCCESS ) {

			// check to see if it needs to be cleaned up
			if( _current->state == KILLED ) {
				// yes - deallocate it
				_cleanup( _current );
				// go back and re-check this queue
				continue;
			}

			_current->state = RUNNING;
			_current->quantum = STD_QUANTUM;
			return;
		} else {
			_kpanic( "_dispatch", "readyq deque status %s",
				 status );
		}

	   } while( 1 );

	}

	_kpanic( "_dispatch", "no non-empty readyq", EMPTY_QUEUE );

}

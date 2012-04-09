/*
** SCCS ID:	@(#)pcbs.c	1.1	4/5/12
**
** File:	pcbs.c
**
** Author:	4003-506 class of 20113
**
** Contributor:
**
** Description:	PCB module
*/

#define	__KERNEL__20113__

#include "headers.h"

#include "queues.h"
#include "pcbs.h"

/*
** PRIVATE DEFINITIONS
*/

/*
** PRIVATE DATA TYPES
*/

/*
** PRIVATE GLOBAL VARIABLES
*/

// Available PCB queue

static Queue *_pcb_free_queue;

/*
** PUBLIC GLOBAL VARIABLES
*/

// PCB variables

Pcb _pcbs[ N_PCBS ];		// all the PCBs in the system
Pid _next_pid;			// next available PID

/*
** PRIVATE FUNCTIONS
*/

/*
** PUBLIC FUNCTIONS
*/


/*
** _pcb_alloc()
**
** allocate a PCB structure
**
** returns a pointer to the PCB, or NULL on failure
*/

Pcb *_pcb_alloc( void ) {
	Pcb *pcb;

	if( _q_remove( _pcb_free_queue, (void **) &pcb ) != SUCCESS ) {
		pcb = NULL;
	} else if( pcb->state != FREE ) {
		_kpanic( "_pcb_alloc", "removed non-FREE pcb", FAILURE );
	}

	return( pcb );
}

/*
** _pcb_dealloc(pcb)
**
** deallocate a pcb, putting it into the list of available pcbs
**
** returns the status from inserting the PCB into the free queue
*/

Status _pcb_dealloc( Pcb *pcb ) {
	Key key;

	// Question: Is it OK to dealloc a NULL pointer?
	if( pcb == NULL ) {
		return( BAD_PARAM );
	}

	key.i = 0;
	pcb->state = FREE;
	return( _q_insert( _pcb_free_queue, (void *)pcb, key ) );

}

/*
** _pcb_init( void )
**
** initializes all process-related data structures
*/

void _pcb_init( void ) {
	int i;
	Status status;

	// init pcbs

	status = _q_alloc( &_pcb_free_queue, NULL );
	if( status != SUCCESS ) {
		_kpanic( "_pcb_init", "PCB queue alloc status %s", status );
	}

	for( i = 0; i < N_PCBS; ++i ) {
		status = _pcb_dealloc( &_pcbs[i] );
		if( status != SUCCESS ) {
			_kpanic( "_pcb_init", "PCB queue insert", status );
		}
	}

	// initial PID

	_next_pid = PID_INIT;

	// report that we have finished

	c_puts( " pcbs" );

}

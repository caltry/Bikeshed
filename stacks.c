/*
** SCCS ID:	@(#)stacks.c	1.1	4/5/12
**
** File:	stacks.c
**
** Author:	4003-506 class of 20113
**
** Contributor:
**
** Description:	Stack module
*/

#define	__KERNEL__20113__

#include "headers.h"

#include "queues.h"
#include "stacks.h"

/*
** PRIVATE DEFINITIONS
*/

/*
** PRIVATE DATA TYPES
*/

/*
** PRIVATE GLOBAL VARIABLES
*/

// User stacks

static Stack _stacks[ N_STACKS ];

// Available Stack queue

static Queue *_stack_free_queue;

/*
** PUBLIC GLOBAL VARIABLES
*/

Stack _system_stack;	// stack for the OS
Uint32 *_system_esp;	// OS stack pointer

/*
** PRIVATE FUNCTIONS
*/

/*
** PUBLIC FUNCTIONS
*/

/*
** _stack_alloc()
**
** allocate a stack structure
**
** returns a pointer to the stack, or NULL on failure
*/

Stack *_stack_alloc( void ) {
	Stack *stack;

	if( _q_remove( _stack_free_queue, (void **) &stack ) != SUCCESS ) {
		stack = NULL;
	}

	return( stack );
}

/*
** _stack_dealloc(stack)
**
** deallocate a stack, putting it into the list of available stacks
**
** returns the status from inserting the PCB into the free queue
*/

Status _stack_dealloc( Stack *stack ) {
	Key key;

	// Is it OK to deallocate a NULL stack?
	if( stack == NULL ) {
		return( BAD_PARAM );
	}

	key.i = 0;
	return( _q_insert( _stack_free_queue, (void *)stack, key ) );

}

/*
** _stack_init( void )
**
** initializes all process-related data structures
*/

void _stack_init( void ) {
	int i;
	Status status;

	// init stacks

	status = _q_alloc( &_stack_free_queue, NULL );
	if( status != SUCCESS ) {
		_kpanic( "_stack_init",
			 "Stack queue alloc status %s",
			 	 status );
	}

	for( i = 0; i < N_STACKS; ++i ) {
		status = _stack_dealloc( &_stacks[i] );
		if( status != SUCCESS ) {
			_kpanic( "_stack_init",
				 "Stack free queue insert status %s",
				  status );
		}
	}

	// report that we have finished

	c_puts( " stacks" );

}

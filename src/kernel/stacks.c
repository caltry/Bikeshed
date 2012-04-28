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

#include "memory/kmalloc.h"

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

// Available Stack queue

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
	Stack *stack = (Stack *)__kmalloc(sizeof(Stack));

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

	// Is it OK to deallocate a NULL stack?
	if( stack == NULL ) {
		return( BAD_PARAM );
	}

	__kfree(stack);

	return SUCCESS;
}

/*
** _stack_init( void )
**
** initializes all process-related data structures
*/

void _stack_init( void ) {

	// init stacks

	// report that we have finished

	c_puts( " stacks" );

}

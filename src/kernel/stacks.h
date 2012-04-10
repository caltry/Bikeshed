/*
** SCCS ID:	@(#)stacks.h	1.1	4/5/12
**
** File:	stacks.h
**
** Author:	4003-506 class of 20113
**
** Contributor:
**
** Description:	Stack module definitions
*/

#ifndef _PROCESSES_H
#define _PROCESSES_H

#include "headers.h"

/*
** General (C and/or assembly) definitions
*/

// size of a user stack (in longwords)

#define	STACK_SIZE	1024

// number of stacks

#define	N_STACKS	N_PROCESSES

#ifndef __ASM__20113__

/*
** Start of C-only definitions
*/

/*
** Types
*/

// user stacks

typedef Uint32		Stack[ STACK_SIZE ];

/*
** Globals
*/

extern Stack _system_stack;	// stack for the OS
extern Uint32 *_system_esp;	// OS stack pointer

/*
** Prototypes
*/

/*
** _stack_alloc()
**
** allocate a Stack structure
**
** returns a pointer to the Stack, or NULL on failure
*/

Stack *_stack_alloc( void );

/*
** _stack_free(stack)
**
** deallocate a stack, putting it into the list of available stacks
**
** returns the status from inserting the Stack into the free queue
*/

Status _stack_dealloc( Stack *stack );

/*
** _stack_init()
**
** initializes all process-related data structures
*/

void _stack_init( void );

#endif

#endif

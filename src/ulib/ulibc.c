/*
** SCCS ID:	@(#)ulibc.c	1.1	4/5/12
**
** File:	ulibc.c
**
** Author:	4003-506 class of 20113
**
** Contributor:
**
** Description:	C implementations of user-level library functions
*/

#include "headers.h"

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

// Status value strings
//
// this is identical to the kernel _kstatus_strings array,
// but is separate to simplify life for people implementing VM.

const char *ustatus_strings[] = {
	"success",		/* SUCCESS */
	"failure",		/* FAILURE */
	"bad parameter",	/* BAD_PARAM */
	"empty queue",		/* EMPTY_QUEUE */
	"not empty queue",	/* NOT_EMPTY_QUEUE */
	"allocation failed",	/* ALLOC_FAILED */
	"not found",		/* NOT_FOUND */
	"no queues",		/* NO_QUEUES */
	"bad priority"		/* BAD_PRIO */
};

/*
** PRIVATE FUNCTIONS
*/

/*
** PUBLIC FUNCTIONS
*/

/*
** prt_status - print a status value to the console
**
** the 'msg' argument should contain a %s where
** the desired status value should be printed
*/

void prt_status( char *msg, Status stat ) {

	if( msg == NULL ) {
		return;
	}

	c_printf( msg, ustatus(stat) );

	if( stat >= STATUS_SENTINEL ) {
		c_printf( "bad code: %d", stat );
	}

}

/*
** spawnp - create a new process running a different program
**		at a specific priority
**
** usage:  status = spawnp( &pid, prio, entry );
**
** returns the PID of the child via the 'pid' parameter on
** success, and the status of the creation attempt
*/

Status spawnp( Pid *pid, Prio prio, void (*entry)(void) ) {
	Pid new;
	Status status, status2;

	// create the process
	status = fork( &new );

	// if that failed, return failure status
	if( status != SUCCESS ) {
		return( status );
	}

	// we have a child; it should do the exec(),
	// and the parent should see success

	if( new == 0 ) {	// we're the child
		// change the process priority
		status = set_priority( prio );
		if( status != SUCCESS ) {
			status2 = get_pid( &new );
			c_printf( "Child pid %d", new );
			prt_status( ", set_priority() status %s\n", status );
			exit();
		}
		status = exec( entry );
		// if we got here, the exec() failed
		status2 = get_pid( &new );
		c_printf( "Child pid %d", new );
		prt_status( ", exec() status %s\n", status );
		exit();
	}

	*pid = new;
	return( SUCCESS );

}

/*
** spawn - create a new process running a different program
**		at standard priority
**
** usage:  status = spawn( &pid, entry );
**
** returns the PID of the child via the 'pid' parameter on
** success, and the status of the creation attempt
*/

Status spawn( Pid *pid, void (*entry)(void) ) {

	// take the easy way out

	return( spawnp(pid,PRIO_STD,entry) );

}

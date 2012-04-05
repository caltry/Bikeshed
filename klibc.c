/*
** SCCS ID:	@(#)klibc.c	1.1	4/5/12
**
** File:	klibc.c
**
** Author:	4003-506 class of 20113
**
** Contributor:
**
** Description:	C implementations of kernel library functions
*/

#define	__KERNEL__20113__

#include "headers.h"

/*
** PUBLIC GLOBAL VARIABLES
*/

// Status value strings
//
// (must track Status definition in types.h)

const char *_kstatus_strings[] = {
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
** _kmemclr - initialize all bytes of a block of memory to zero
**
** usage:  _kmemclr( buffer, length )
*/

void _kmemclr( void *buffer, Uint32 length ) {
	register unsigned char *buf = (unsigned char *)buffer;
	register Uint32 num = length;

	while( num-- ) {
		*buf++ = 0;
	}

}

/*
** _kmemcpy - copy a block from one place to another
**
** usage:  _kmemcpy( dest, src, length )
**
** may not correctly deal with overlapping buffers
*/

void _kmemcpy( void *destination, void *source, Uint32 length ) {
	register unsigned char *dst = (unsigned char *)destination;
	register unsigned char *src = (unsigned char *)source;
	register Uint32 num = length;

	while( num-- ) {
		*dst++ = *src++;
	}

}


/*
** _kpanic - kernel-level panic routine
**
** usage:  _kpanic( mod, msg, code )
**
** Prefix routine for __panic() - can be expanded to do other things
** (e.g., printing a stack traceback)
**
** 'mod' argument is always printed; 'msg' argument is printed
** if it isn't NULL, followed by a newline
**
** 'msg' argument should contain a %s format code if the 'code'
** argument is to be printed as part of the panic message
**
** if 'code' is invalid, it will be printed in integer form
*/

void _kpanic( char *mod, char *msg, Status code ) {

	c_puts( "\n\n***** KERNEL PANIC *****\n\n" );
	c_printf( "Module: %s\n", mod );
	if( msg != NULL ) {
		c_printf( msg, _kstatus(code) );
		c_putchar( '\n' );
	}
	if( code >= STATUS_SENTINEL ) {
		c_printf( "*** bad code %d\n", code );
	}

	//
	// This might be a good place to do a stack frame
	// traceback
	//

	__panic( "KERNEL PANIC" );

}

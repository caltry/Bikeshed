/*
** SCCS ID:	@(#)klib.h	1.1	4/5/12
**
** File:	klib.h
**
** Author:	4003-506 class of 20113
**
** Contributor:
**
** Description:	Kernel library definitions
*/

#ifndef _KLIB_H
#define _KLIB_H

#include "headers.h"

/*
** General (C and/or assembly) definitions
*/

#ifndef __ASM__20113__

/*
** Start of C-only definitions
*/

// pseudo-function:  return a string describing a Status value
//
// (must track Status definition in types.h)

#define	_kstatus(n) \
	((n) >= STATUS_SENTINEL ? "bad status value" : _kstatus_strings[(n)])

/*
** Types
*/

/*
** Globals
*/

// Status value strings

extern const char *_kstatus_strings[];

/*
** Prototypes
*/

/*
** _get_ebp - return current contents of EBP at the time of the call
**
** Could be used, e.g., by _kpanic() to print a traceback
*/

Uint32 _get_ebp( void );

/*
** _kmemclr - initialize all bytes of a block of memory to zero
**
** usage:  _kmemclr( buffer, length )
*/

void _kmemclr( void *buffer, Uint32 length );

/*
** _kmemcpy - copy a block from one place to another
**
** usage:  _kmemcpy( dest, src, length )
**
** may not correctly deal with overlapping buffers
*/

void _kmemcpy( void *destination, void *source, Uint32 length );

/*
** _kpanic - kernel-level panic routine
**
** usage:  _kpanic( mod, msg, code )
**
** Prefix routine for __panic() - can be expanded to do other things
** (e.g., printing a stack traceback)
**
** 'msg' argument should contain a %s format code if the 'code'
** argument is to be printed as part of the panic message
*/

void _kpanic( char *mod, char *msg, Status code );

#endif

#endif

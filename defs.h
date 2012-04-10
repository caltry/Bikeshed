/*
** SCCS ID:	@(#)defs.h	1.1	4/5/12
**
** File:	defs.h
**
** Author:	4003-506 class of 20113
**
** Contributor:
**
** Description:	General definitions
*/

#ifndef _DEFS_H
#define _DEFS_H

#include "headers.h"

/*
** General (C and/or assembly) definitions
*/

// Null pointer (defined the correct way)

#define	NULL		0

// Maximum number of simultaneous user processes

#define	N_PROCESSES	25

#ifndef __ASM__20113__

/*
** Start of C-only definitions
*/

/*
** Types
*/
typedef int int32;
typedef unsigned int uint32;

typedef char int8;
typedef unsigned char uint8;

/*
** Globals
*/

/*
** Prototypes
*/

#endif

#endif

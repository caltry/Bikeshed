/*
** SCCS ID:	@(#)types.h	1.1	4/5/12
**
** File:	types.h
**
** Author:	4003-506 class of 20113
**
** Contributor:
**
** Description:	General type definitions
*/

#ifndef _TYPES_H
#define _TYPES_H

#include "headers.h"

#ifndef __ASM__20113__

/*
** Types
*/

// generic integer types

typedef unsigned int	Uint;

// size-specific integer types

typedef long		Int32;
typedef unsigned long	Uint32;

typedef short		Int16;
typedef unsigned short	Uint16;

typedef char		Int8;
typedef unsigned char	Uint8;

// Status type
//
// This is shared between the kernel and user levels
//
// LAST ENTRY MUST BE STATUS_SENTINEL

typedef enum stat {
	SUCCESS=0, FAILURE, BAD_PARAM, EMPTY_QUEUE, NOT_EMPTY_QUEUE,
	ALLOC_FAILED, NOT_FOUND, NO_QUEUES, BAD_PRIO,
	STATUS_SENTINEL
} Status;

// Key type (used for ordering things)

typedef union key {
        int i;
        Uint32 u;
        void *v;
} Key;

// Process ID

typedef	Uint16		Pid;

// Time (kept in clock ticks)

typedef	Uint32		Time;

#endif

#endif

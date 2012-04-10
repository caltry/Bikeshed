/*
** SCCS ID:	@(#)clock.h	1.1	4/5/12
**
** File:	clock.h
**
** Author:	4003-506 class of 20113
**
** Contributor:
**
** Description:	Clock module definitions
*/

#ifndef _CLOCK_H
#define _CLOCK_H

#include "headers.h"

/*
** General (C and/or assembly) definitions
*/

// clock interrupts per second

#define	CLOCK_FREQUENCY		1000

// standard process quantum, in ticks

#define	STD_QUANTUM		10

#ifndef __ASM__20113__

/*
** Start of C-only definitions
*/

// pseudo-function to convert seconds to ticks

#define	SECONDS_TO_TICKS(n)	((n) * CLOCK_FREQUENCY)

// pseudo-function to convert ticks to seconds

#define	TICKS_TO_SECONDS(n)	((n) / CLOCK_FREQUENCY)

/*
** Types
*/

/*
** Globals
*/

// system time

extern Time _system_time;

/*
** Prototypes
*/

/*
** _clock_init()
**
** initialize the clock module
*/

void _clock_init( void );

/*
** _isr_clock(vector,code)
**
** clock ISR
*/

void _isr_clock( int vector, int code );

#endif

#endif

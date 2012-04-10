/*
** SCCS ID:	@(#)scheduler.h	1.1	4/5/12
**
** File:	scheduler.h
**
** Author:	4003-506 class of 20113
**
** Contributor:
**
** Description:	Scheduler module definitions
*/

#ifndef _SCHEDULER_H
#define _SCHEDULER_H

#include "headers.h"

/*
** General (C and/or assembly) definitions
*/

#ifndef __ASM__20113__

/*
** Start of C-only definitions
*/

#include "pcbs.h"
#include "queues.h"

// number of ready queues:  one per priority level

#define	N_READYQ	N_PRIOS

/*
** Types
*/

/*
** Globals
*/

extern Pcb *_current;	// the current process

// the array of ready queues for our MLQ structure

extern Queue *_ready[ N_READYQ ];

/*
** Prototypes
*/

/*
** _sched_init()
**
** initialize the scheduler module
*/

void _sched_init( void );

/*
** _sched(pcb)
**
** schedule a process for execution according to is priority
**
** returns the status of the insertion into the ready queue
*/

Status _sched( Pcb *pcb );

/*
** _dispatch()
**
** give the CPU to a process
*/

void _dispatch( void );

#endif

#endif

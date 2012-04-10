/*
** SCCS ID:	@(#)pcbs.h	1.1	4/5/12
**
** File:	pcbs.h
**
** Author:	4003-506 class of 20113
**
** Contributor:
**
** Description:	PCB module definitions
*/

#ifndef _PCBS_H
#define _PCBS_H

#include "headers.h"

/*
** General (C and/or assembly) definitions
*/

// Number of PCBS

#define	N_PCBS		N_PROCESSES

// Process states

#define	FREE		0
#define	NEW		1
#define	READY		2
#define	RUNNING		3
#define	SLEEPING	4
#define	BLOCKED		5
#define	KILLED		6

#define	N_STATES	7

// Process priorities

#define	PRIO_HIGH	0
#define	PRIO_STD	1
#define	PRIO_LOW	2
#define	PRIO_IDLE	3

#define	N_PRIOS		4

// PID of the initial user process

#define	PID_INIT	1

// ARG(p) - access argument from user stack of process
//
// ARG(p)[0]: return address
// ARG(p)[n]: argument #n

#define ARG(p)  ((Uint32 *) ((p)->context + 1))

// RET(p) - access return value register in process context

#define RET(p)  ((p)->context->eax)

#ifndef __ASM__20113__

/*
** Start of C-only definitions
*/

#include "clock.h"
#include "stacks.h"

/*
** Types
*/

// process states

typedef Uint8		State;

// process priorities

typedef Uint8		Prio;


// process context structure
//
// NOTE:  the order of data members here depends on the
// register save code in isr_stubs.S!!!!

typedef struct context {
	Uint32 ss;
	Uint32 gs;
	Uint32 fs;
	Uint32 es;
	Uint32 ds;
	Uint32 edi;
	Uint32 esi;
	Uint32 ebp;
	Uint32 esp;
	Uint32 ebx;
	Uint32 edx;
	Uint32 ecx;
	Uint32 eax;
	Uint32 vector;
	Uint32 code;
	Uint32 eip;
	Uint32 cs;
	Uint32 eflags;
} Context;

// process control block
//
// members are ordered by size

typedef struct pcb {
	// four-byte fields
	Context		*context;	// process context
	Stack		*stack;		// this process' stack
	Time		wakeup;		// wakeup time for sleeping process
	// two-byte fields
	Pid		pid;		// our processid
	Pid		ppid;		// who created us
	// one-byte fields
	State		state;		// current process state
	Prio		priority;	// current process priority
	Uint8		quantum;	// remaining process quantum
} Pcb;

/*
** Globals
*/

extern Pcb _pcbs[];		// all PCBs in the system
extern Pid _next_pid;		// next available PID

/*
** Prototypes
*/

/*
** _pcb_alloc()
**
** allocate a PCB structure
**
** returns a pointer to the PCB, or NULL on failure
*/

Pcb *_pcb_alloc( void );

/*
** _pcb_free(pcb)
**
** deallocate a pcb, putting it into the list of available pcbs
**
** returns the status from inserting the PCB into the free queue
*/

Status _pcb_dealloc( Pcb *pcb );

/*
** _pcb_init()
**
** initializes all process-related data structures
*/

void _pcb_init( void );

#endif

#endif

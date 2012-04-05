/*
** SCCS ID:	@(#)users.h	1.1	4/5/12
**
** File:	users.h
**
** Author:	4003-506 class of 20113
**
** Contributor:
**
** Description:	User routines
*/

#ifndef _USER_H
#define _USER_H

#include "headers.h"

/*
** General (C and/or assembly) definitions
*/

// delay loop counts

#define	DELAY_LONG	100000000
#define	DELAY_STD	  2500000
#define	DELAY_ALT	  4500000

#ifndef __ASM__20113__

/*
** Start of C-only definitions
*/

/*
** User process controls.
**
** The comment field of these definitions contains a list of the
** system calls this user process uses ("spawn" performs "fork"
** and "exec"; "spawnp" performs "fork", "set_priority", and "exec";
** "sleep" performs "msleep").
**
** To spawn a specific user process from the initial process,
** uncomment its entry in this list.
*/

#define	SPAWN_A   // write, exit
#define	SPAWN_B   // write, exit
#define	SPAWN_C   // write, exit
#define	SPAWN_D   // write, spawn, exit
#define	SPAWN_E   // write, get_pid, sleep, exit
#define	SPAWN_F   // write, get_pid, sleep, exit
#define	SPAWN_G   // write, get_pid, sleep, exit
#define	SPAWN_H   // write
// no user i
#define	SPAWN_J   // write, spawn, exit
#define	SPAWN_K   // write, sleep, spawn, exit
#define	SPAWN_L   // write, sleep, spawn, get_time, exit
#define	SPAWN_M   // write, spawnp, exit
#define	SPAWN_N   // write, spawnp, exit
// no user O
#define	SPAWN_P   // write, sleep, get_time
#define	SPAWN_Q   // write, bogus, exit
#define	SPAWN_R   // write, read, sleep, exit
#define	SPAWN_S   // write, sleep, forever
#define	SPAWN_T   // write, get_prio, set_priority, exit
// no user U
// no user V

/*
** Users W-Z are spawned from other processes; they
** should never be spawned directly.
*/

// user W:  write, sleep, getpid, exit
// user X:  write, get_pid, exit
// user Y:  write, sleep, exit
// user Z:  write, exit


/*
** Prototypes for externally-visible routines
*/

/*
** init - initial user process
**
** becomes the idle process after spawning the other user processes
*/

void init( void );

#endif

#endif

/*
** SCCS ID:	@(#)ulib.h	1.1	4/5/12
**
** File:	ulib.h
**
** Author:	4003-506 class of 20113
**
** Contributor:
**
** Description:	User-level library definitions
*/

#ifndef _ULIB_H
#define _ULIB_H

#include "headers.h"

/*
** General (C and/or assembly) definitions
*/

#ifndef __ASM__20113__

/*
** Start of C-only definitions
*/

#include "types.h"
#include "clock.h"
#include "pcbs.h"

// pseudo-function:  sleep for a specified number of seconds

#define	sleep(n)	msleep(SECONDS_TO_TICKS(n))

// pseudo-function:  return a string describing a Status value

#define ustatus(n) \
	((n) >= STATUS_SENTINEL ? "bad status value" : ustatus_strings[(n)])

/*
** Types
*/

/*
** Globals
*/

// Status value strings

extern const char *ustatus_strings[];

/*
** Prototypes
*/

/*
** fork - create a new process
**
** usage:  status = fork(&pid);
**
** returns:
**      PID of new process via the info pointer (in parent)
**      0 via the info pointer (in child)
**      status of the creation attempt
*/

Status fork(Pid *pid);

/*
** exit - terminate the calling process
**
** usage:	exit();
**
** does not return
*/

Status exit(void);

/*
** read - read a single character from the SIO
**
** usage:	status = read(&buf);
**
** blocks the calling routine if there is no character to return
**
** returns:
**      the character via the info pointer
**      status of the operation
*/

Status read(int *buf);

/*
** write - write a single character to the SIO
**
** usage:	status = write(ch);
**
** returns:
**      status of the operation
*/

Status write(char buf);

/*
** msleep - put the current process to sleep for some length of time
**
** usage:	status = msleep(ms);
**
** if the sleep time (in milliseconds) is 0, just preempts the process;
** otherwise, puts it onto the sleep queue for the specified length of
** time
**
** returns:
**      status of the sleep attempt
*/

Status msleep(Uint32 ms);

/*
** kill - terminate a process with extreme prejudice
**
** usage:	status kill(pid);
**
** returns:
**      status of the termination attempt
*/

Status kill(Pid pid);

/*
** get_priority - retrieve the priority of the current process
**
** usage:	status = get_priority(&prio);
**
** returns:
**      the process' priority via the info pointer
**      SUCCESS
*/

Status get_priority(Prio *prio);

/*
** get_pid - retrieve the PID of the current process
**
** usage:	status = get_pid(&pid);
**
** returns:
**      the process' pid via the info pointer
**      SUCCESS
*/

Status get_pid(Pid *pid);

/*
** get_ppid - retrieve the parent PID of the current process
**
** usage:	status = get_ppid(&pid);
**
** returns:
**      the process' parent's pid via the info pointer
**      SUCCESS
*/

Status get_ppid(Pid *pid);

/*
** get_time - retrieve the current system time
**
** usage:	status = get_time(&time);
**
** returns:
**      the process' pid via the info pointer
**      SUCCESS
*/

Status get_time(Time *time);

/*
** get_state - retrieve the state of the current process
**
** usage:	status = get_state(&state);
**
** returns:
**      the process' state via the info pointer
**      SUCCESS
*/

Status get_state(State *state);

/*
** set_priority - change the priority of the current process
**
** usage:	status = set_priority(prio);
**
** returns:
**      success of the change attempt
*/

Status set_priority(Prio prio);

/*
** set_time - change the current system time
**
** usage:	status = set_time(time);
**
** returns:
**      SUCCESS
*/

Status set_time(Time time);

/*
** exec - replace a process with a different program
**
** usage:	status = exec(entry);
**
** returns:
**	does not return (if the attempt succeeds)
**      failure status of the replacement attempt (if the attempt fails)
*/

Status exec(void (*entry)(void));

/*
** bogus - a bogus system call, for testing our syscall ISR
**
** usage:	bogus();
*/

void bogus( void );

/*
** prt_status - print a status value to the console
**
** usage:  prt_status(msg,code);
**
** the 'msg' argument should contain a %s where
** the desired status value should be printed
*/

void prt_status( char *msg, Status stat );

/*
** spawn - create a new process running a different program
**		at standard priority
**
** usage:  status = spawn( &pid, entry );
*/

Status spawn( Pid *pid, void (*entry)(void) );

/*
** spawnp - create a new process running a different program at
**		a specific process priority
**
** usage:  status = spawnp( &pid, prio, entry );
*/

Status spawnp( Pid *pid, Prio prio, void (*entry)(void) );

#endif

#endif

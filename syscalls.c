/*
** SCCS ID:	@(#)syscalls.c	1.1	4/5/12
**
** File:	syscalls.c
**
** Author:	4003-506 class of 20113
**
** Contributor:
**
** Description:	System call module
*/

#define	__KERNEL__20113__

#include "headers.h"

#include "pcbs.h"
#include "scheduler.h"
#include "sio.h"
#include "syscalls.h"
#include "system.h"

#include "startup.h"

/*
** PRIVATE DEFINITIONS
*/

/*
** PRIVATE DATA TYPES
*/

/*
** PRIVATE GLOBAL VARIABLES
*/

// system call jump table

static void (*_syscall_tbl[ N_SYSCALLS ])(Pcb *);

/*
** PUBLIC GLOBAL VARIABLES
*/

// queue of sleeping processes

Queue *_sleeping;

/*
** PRIVATE FUNCTIONS
*/

/*
** Second-level syscall handlers
**
** All have this prototype:
**
**	static void _sys_NAME( Pcb * );
**
** Most syscalls return a Status value to the user calling function.
** Those which return additional information from the system have as
** their first user-level argument a pointer (called the "info pointer"
** below) to a variable into which the information is to be placed.
*/

/*
** _sys_fork - create a new process
**
** implements:	Status fork(Pid *pid);
**
** returns:
**	PID of new process via the info pointer (in parent)
**	0 via the info pointer (in child)
**	status of the creation attempt
*/

static void _sys_fork( Pcb *pcb ) {
	Pcb *new;
	int diff;
	Uint32 *ptr;
	Status status;

	// allocate a pcb for the new process

	new = _pcb_alloc();
	if( new == NULL ) {
		RET(pcb) = FAILURE;
		return;
	}

	// duplicate the parent's pcb

	_kmemcpy( (void *)new, (void *)pcb, sizeof(Pcb) );

	// allocate a stack for the new process

	new->stack = _stack_alloc();
	if( new->stack == NULL ) {
		RET(pcb) = FAILURE;
		_cleanup( new );
		return;
	}

	// duplicate the parent's stack

	_kmemcpy( (void *)new->stack, (void *)pcb->stack, sizeof(Stack));

	// fix the pcb fields that should be unique to this process

	new->pid = _next_pid++;
	new->ppid = pcb->pid;
	new->state = NEW;

	/*
	** We duplicated the parent's stack contents, which means that
	** the child's ESP and EBP are still pointing into the parent's
	** stack.  Fix these, and also fix the child's context pointer.
        **
        ** We have to change EBP because that's how the compiled code for
        ** the user process accesses its local variables.  If we didn't
        ** change this, as soon as the child was dispatched, it would
        ** start to stomp on the local variables in the parent's stack.
	** We also have to fix the EBP chain in the child process.
        **
        ** None of this would be an issue if we were doing "real" virtual
        ** memory, as we would be talking about virtual addresses here rather
        ** than physical addresses, and all processes would share the same
        ** virtual address space layout.
	**
	** First, determine the distance (in bytes) between the two
	** stacks.  This is the adjustment value we must add to the
	** three pointers to correct them.
	*/

	diff = (void *) new->stack - (void *) pcb->stack;

	// adjust the context pointer, esp, and ebp

	new->context = (Context *) ( (void *) new->context + diff );

	new->context->esp += diff;
	new->context->ebp += diff;

	/*
        ** Next, we must fix the EBP chain in the child.  This is necessary
        ** in the situation where the fork() occurred in a nested function
	** call sequence; we fixed EBP, but the "saved" EBP in the stack
	** frame is pointing to the calling function's frame in the parent's
	** stack, not the child's stack.
	**
	** We are guaranteed that the chain of frames ends at the user
	** process' main routine, because exec() will initialize EBP for
	** the process to 0, and the entry prologue code in the main
	** routine will push EBP, ensuring a NULL pointer in the chain.
        */

	// start at the current frame

	ptr = (Uint32 *) new->context->ebp;

	// follow the chain of frame pointers to its end
	while( *ptr != 0 ) {
		// update the back link from this frame to the previous
		*ptr = (Uint32) ((void *) *ptr + diff );
		// follow the updated link
		ptr = (Uint32 *) *ptr;
	}

	// assign the PID return values for the two processes

	ptr = (Uint32 *) (ARG(pcb)[1]);
	*ptr = new->pid;

	ptr = (Uint32 *) ( ((void *)(ARG(new)[1])) + diff );
	*ptr = 0;

	/*
	** Philosophical issue:  should the child run immediately, or
	** should the parent continue?
	**
	** We take the path of least resistance (work), and opt for the
	** latter; we schedule the child, and let the parent continue.
	*/

	status = _sched( new );
	if( status != SUCCESS ) {
		// couldn't schedule the child, so deallocate it
		RET(pcb) = FAILURE;
		_cleanup( new );
	} else {
		// indicate success for both processes
		RET(pcb) = SUCCESS;
		RET(new) = SUCCESS;
	}

}

/*
** _sys_exit - terminate the calling process
**
** implements:	void exit();
**
** does not return
*/

static void _sys_exit( Pcb *pcb ) {

	// deallocate all the OS data structures

	_cleanup( pcb );

	// select a new current process

	_dispatch();

}

/*
** _sys_read - read a single character from the SIO
**
** implements:	Status read(int *buf);
**
** blocks the calling routine if there is no character to return
**
** returns:
**	the character via the info pointer
**	status of the operation
*/

static void _sys_read( Pcb *pcb ) {
	Key key;
	int ch;
	int *ptr;
	Status status;

	// try to get the next character

	ch = _sio_readc();

	// if there was a character, return it to the process;
	// otherwise, block the process until one comes in

	if( ch >= 0 ) {

		ptr = (int *) (ARG(pcb)[1]);
		*ptr = ch;
		RET(pcb) = SUCCESS;

	} else {

		// no character; put this process on the
		// serial i/o input queue

		_current->state = BLOCKED;

		key.u = _current->pid;
		status = _q_insert( _reading, (void *) _current, key );
		if( status != SUCCESS ) {
			_kpanic( "_sys_read", "insert status %s", status );
		}

		// select a new current process

		_dispatch();

	}


}

/*
** _sys_write - write a single character to the SIO
**
** implements:	Status write(char buf);
**
** returns:
**	status of the operation
*/

static void _sys_write( Pcb *pcb ) {
	int ch = ARG(pcb)[1];

	// this is almost insanely simple, but it does separate
	// the low-level device access fromm the higher-level
	// syscall implementation

	_sio_writec( ch );

	RET(pcb) = SUCCESS;

}

/*
** _sys_msleep - put the current process to sleep for some length of time
**
** implements:	Status msleep(Uint32 ms);
**
** if the sleep time (in milliseconds) is 0, just preempts the process;
** otherwise, puts it onto the sleep queue for the specified length of
** time
**
** returns:
**	status of the sleep attempt
*/

static void _sys_msleep( Pcb *pcb ) {
	Key wakeup;
	Uint32 time;
	Status status;

	// retrieve the sleep time from the user

	time = ARG(pcb)[1];

	// if the user says "sleep for no seconds", we just preempt it;
	// otherwise, we put it on the sleep queue

	if( time == 0 ) {

		status = _sched( pcb );
		if( status != SUCCESS ) {
			RET(pcb) = FAILURE;
			c_printf( "msleep(%u), pid %u: can't schedule, status %s\n",
				  time, pcb->pid, _kstatus(status) );
			return;
		}

	} else {

		// calculate the wakeup time
		wakeup.u = _system_time + time;

		// do the insertion
		status = _q_insert( _sleeping, (void *) pcb, wakeup );
		if( status != SUCCESS ) {
			RET(pcb) = FAILURE;
			c_printf( "msleep(%u), pid %u: can't sleep, status %s\n",
				  time, pcb->pid, _kstatus(status) );
			return;
		}

	}

	// in either case, we return SUCCESS to the user and
	// dispatch a new process

	RET(pcb) = SUCCESS;

	_dispatch();

}

/*
** _sys_kill - terminate a process with extreme prejudice
**
** implements:	Status kill(Pid pid);
**
** returns:
**	status of the termination attempt
*/

static void _sys_kill( Pcb *pcb ) {
	int i;
	Uint32 pid;

	// figure out who we are going to kill

	pid = ARG(pcb)[1];

	// locate the victim by brute-force scan of the PCB array

	for( i = 0; i < N_PCBS; ++i ) {

		// this is the victim IFF:
		//	the PIDs match, and
		//	the PCB is *not* marked as available

		if( _pcbs[i].pid == pid &&
		    _pcbs[i].state != FREE ) {
			// got it!
		    	_pcbs[i].state = KILLED;
			RET(pcb) = SUCCESS;
			return;
		}
	}

	// we could not find the specified PID, or we found it
	// but it isn't currently alive

	RET(pcb) = NOT_FOUND;

}

/*
** _sys_get_priority - retrieve the priority of the current process
**
** implements:	Status get_priority(Prio *prio);
**
** returns:
**	the process' priority via the info pointer
**	SUCCESS
*/

static void _sys_get_priority( Pcb *pcb ) {

	RET(pcb) = SUCCESS;
	*((Uint32 *)(ARG(pcb)[1])) = pcb->priority;

}

/*
** _sys_get_pid - retrieve the PID of the current process
**
** implements:	Status get_pid(Pid *pid);
**
** returns:
**	the process' pid via the info pointer
**	SUCCESS
*/

static void _sys_get_pid( Pcb *pcb ) {

	RET(pcb) = SUCCESS;
	*((Uint32 *)(ARG(pcb)[1])) = pcb->pid;

}

/*
** _sys_get_ppid - retrieve the parent PID of the current process
**
** implements:	Status get_ppid(Pid *pid);
**
** returns:
**	the process' parent's pid via the info pointer
**	SUCCESS
*/

static void _sys_get_ppid( Pcb *pcb ) {

	RET(pcb) = SUCCESS;
	*((Uint32 *)(ARG(pcb)[1])) = pcb->ppid;

}

/*
** _sys_get_time - retrieve the current system time
**
** implements:	Status get_time(Time *time);
**
** returns:
**	the process' pid via the info pointer
**	SUCCESS
*/

static void _sys_get_time( Pcb *pcb ) {

	RET(pcb) = SUCCESS;
	*((Uint32 *)(ARG(pcb)[1])) = _system_time;

}

/*
** _sys_get_state - retrieve the state of the current process
**
** implements:	Status get_state(State *state);
**
** returns:
**	the process' state via the info pointer
**	SUCCESS
*/

static void _sys_get_state( Pcb *pcb ) {

	RET(pcb) = SUCCESS;
	*((Uint32 *)(ARG(pcb)[1])) = pcb->state;

}

/*
** _sys_set_priority - change the priority of the current process
**
** implements:	Status set_priority(Prio prio);
**
** returns:
**	success of the change attempt
*/

static void _sys_set_priority( Pcb *pcb ) {
	Prio prio;

	// retrieve the desired priority

	prio = (Prio) ARG(pcb)[1];

	// if the priority is valid, do the change;
	// otherwise, report the failure

	if( prio < N_PRIOS ) {
		pcb->priority = prio;
		RET(pcb) = SUCCESS;
	} else {
		RET(pcb) = BAD_PARAM;
	}

}

/*
** _sys_set_time - change the current system time
**
** implements:	Status set_time(Time time);
**
** returns:
**	SUCCESS
*/

static void _sys_set_time( Pcb *pcb ) {

	// this is extremely simple, but disturbingly powerful

	_system_time = (Time) ARG(pcb)[1];
	RET(pcb) = SUCCESS;

}

/*
** _sys_exec - replace a process with a different program
**
** implements:	Status exec(void (*entry)(void));
**
** returns:
**	failure status of the replacement attempt
**		(only if the attempt fails)
*/

static void _sys_exec( Pcb *pcb ) {
	Status status;

	// invoke the common code for process creation

	status = _create_process( pcb, ARG(pcb)[1] );

	// we only need to assign this if the creation failed
	// for some reason - otherwise, this process never
	// "returns" from the syscall

	if( status != SUCCESS ) {
		RET(pcb) = status;
	}

}


/*
** PUBLIC FUNCTIONS
*/


/*
** _syscall_init()
**
** initialize the system call module
*/

void _syscall_init( void ) {
	Status status;

	/*
	** Create the sleep queue.
	**
	** It is sorted in ascending order by wakeup time.
	*/

	status = _q_alloc( &_sleeping, _comp_ascend_uint );
	if( status != SUCCESS ) {
		_kpanic( "_syscall_init", "sleepq alloc status %s\n", status );
	}

	/*
	** Set up the syscall jump table.  We do this here
	** to ensure that the association between syscall
	** code and function address is correct even if the
	** codes change.
	*/

	_syscall_tbl[ SYS_fork ]          = _sys_fork;
	_syscall_tbl[ SYS_exec ]          = _sys_exec;
	_syscall_tbl[ SYS_exit ]          = _sys_exit;
	_syscall_tbl[ SYS_msleep ]        = _sys_msleep;
	_syscall_tbl[ SYS_read ]          = _sys_read;
	_syscall_tbl[ SYS_write ]         = _sys_write;
	_syscall_tbl[ SYS_kill ]          = _sys_kill;
	_syscall_tbl[ SYS_get_priority ]  = _sys_get_priority;
	_syscall_tbl[ SYS_get_pid ]       = _sys_get_pid;
	_syscall_tbl[ SYS_get_ppid ]      = _sys_get_ppid;
	_syscall_tbl[ SYS_get_state ]     = _sys_get_state;
	_syscall_tbl[ SYS_get_time ]      = _sys_get_time;
	_syscall_tbl[ SYS_set_priority ]  = _sys_set_priority;
	_syscall_tbl[ SYS_set_time ]      = _sys_set_time;

//	these are syscalls we elected not to implement
//	_syscall_tbl[ SYS_set_pid ]    = _sys_set_pid;
//	_syscall_tbl[ SYS_set_ppid ]   = _sys_set_ppid;
//	_syscall_tbl[ SYS_set_state ]  = _sys_set_state;

	// report that we're done

	c_puts( " syscalls" );

}

/*
** _isr_syscall(vector,code)
**
** Common handler for the system call module.  Selects
** the correct second-level routine to invoke based on
** the contents of EAX.
**
** The second-level routine is invoked with a pointer to
** the PCB for the process.  It is the responsibility of
** that routine to assign all return values for the call.
*/

void _isr_syscall( int vector, int code ) {
	Uint num;

	// sanity check - verify that there is a current process

	if( _current == NULL ) {
		_kpanic( "_isr_syscall", "null _current", FAILURE );
	}

	// also, make sure it actually has a context

	if( _current->context == NULL ) {
		_kpanic( "_isr_syscall", "null _current context", FAILURE );
	}

	// retrieve the syscall code from the process

	num = RET(_current);

	// verify that it's legal - if not, force an exit

	if( num >= N_SYSCALLS ) {
		c_printf( "syscall: pid %d called %d\n",
			  _current->pid, num );
		num = SYS_exit;
	}

	// call the handler

	(*_syscall_tbl[num])( _current );

	// tell the PIC we're done

	__outb( PIC_MASTER_CMD_PORT, PIC_EOI );

}

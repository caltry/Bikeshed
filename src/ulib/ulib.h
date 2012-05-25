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

/*
** General (C and/or assembly) definitions
*/

#define DELAY_STD 2500000

#ifndef __ASM__20113__

/*
** Start of C-only definitions
*/

#include "types.h"
#include "kernel/pcbs.h"

// pseudo-function:  sleep for a specified number of seconds

#define	sleep(n)	msleep(SECONDS_TO_TICKS(n))

// pseudo-function:  return a string describing a Status value

#define ustatus(n) \
	((n) >= STATUS_SENTINEL ? "bad status value" : ustatus_strings[(n)])

/*
** Types
*/

typedef enum {
	FS_E_OK         = 0,	// No error
	FS_E_NOFILE     = 1,	// File not found
	FS_E_BADFD      = 2,	// Not a valid file descriptor
	FS_E_IO         = 3,	// An I/O error occurred.
	FS_E_NOT_FQN    = 4,	// The path give is not fully qualified
} FSStatus;

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
** usage:	status = exec("/path/to/program");
**
** returns:
**	does not return (if the attempt succeeds)
**      failure status of the replacement attempt (if the attempt fails)
*/

Status exec(const char* program_path);

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
** usage:  status = spawn( &pid, "/path/to/program" );
*/

Status spawn( Pid *pid, const char* program_path );

/*
** spawnp - create a new process running a different program at
**		a specific process priority
**
** usage:  status = spawnp( &pid, prio, "/path/to/program" );
*/

Status spawnp( Pid *pid, Prio prio, const char* program_path );


/*
** sem_init - Initializes a semaphore
**
** usage:	status = sem_init(&sem);
*/

Status sem_init( Sem *sem );

/*
** sem_destroy - Destroys a semaphore
**
** usage:	status = sem_destroy(sem);
*/

Status sem_destroy( Sem sem );

/*
** sem_post - Increments a semaphore
**
** usage:	status = sem_post(sem);
*/

Status sem_post( Sem sem );

/*
** sem_wait - Waits and decrements a semaphore
**
** usage:	status = sem_wait(sem);
*/

Status sem_wait( Sem sem );

/*
** sem_try_wait - Attempts to decrement a semaphore
**
** usage:	status = sem_try_wait(sem);
*/

Status sem_try_wait( Sem sem );

/*
** lock_init - Creates a lock
*/
Status lock_init( Lock* lock );

/*
** lock_destroy - Destroys a lock
*/
Status lock_destroy( Lock lock );

/*
** lock_lock - Locks a semaphore with the given locking mode.
**
** usage:	status = lock_lock( lock, LOCK_READ )
*/
Status lock_lock( Lock lock, LockMode mode );

/*
** lock_unlock - Unlocks a semaphore with the given locking mode.
**
** usage:	status = lock_unlock( lock, LOCK_WRITE )
*/
Status lock_unlock( Lock lock, LockMode mode );

/*
** message_send - Sends a message to another process.
**
** usage:   status = message_send( pid, (void*) data, size );
*/
Status message_send( Pid pid, void *data, Uint32 size );

/*
** message_receive - Receives a message. Will wait for a message to arrive. 
**
** usage:   status = message_receive( &fromPid, &data, &size );
*/
Status message_receive( Pid *fromPid, void **data, Uint32 *size );

/*
** message_try_receive - Receives a message. Returns NOT_FOUND if there is no message waiting.
**
** usage:   status = message_()
*/
Status message_try_receive( Pid *fromPid, void **data, Uint32 *size );

/*
** message_has_message - Checks if there is a message waiting for the process. Returns NOT_FOUND 
**  if there is no message waitng. SUCCESS if there is.
**
** usage:   status = message_()
*/
Status message_has_message( void );

void writef(const char *fmt, ...);

/*
 *
 * Read _nbytes_ from a file (given its _path_) into a _buf_. Reading begins at
 * _offset_ bytes into the file. Stores the number of _bytes_read_.
 *
 * Returns:
 * 	FS_E_OK     If we read any number of bytes.
 * 	FS_E_NOFILE If there is no file with that name.
 * 	FS_E_IO	    If there was some I/O error.
 */
FSStatus fs_read
	(const char *path,
	void *buf,
	Uint32 *bytes_read,
	Uint32 offset,
	Uint32 nbytes);

/*
 * From the _offset_, write _nbytes_ from _buf_ into a file at _path_. Stores
 * the number of _bytes_read_.
 *
 * Returns:
 * 	E_OK           If we wrote any number of bytes.
 * 	FS_E_NOFILE    If there is no file with that name.
 * 	FS_E_BAD_START If the initial offset into the file doesn't yet exist.
 * 	E_IO           If there was some I/O error.
 */
FSStatus fs_write
	(const char *path,
	void *buf,
	Uint32 *bytes_written,
	Uint32 offset,
	Uint32 nbytes);

#endif

#endif

/*
** File:	locks.c
**
** Author:	Chris Hossenlopp <crh5026@rit.edu>
**
** Contributor:
**
** Description:	Locks module.
*/

#define	__KERNEL__20113__

#include "headers.h"
#include "locks.h"
#include "scheduler.h"
#include "queues.h"
#include "klib.h"
#include "memory/kmalloc.h"

#define TRACE_LOCKS
#ifdef TRACE_LOCKS
#	define TRACE(...) c_printf(__VA_ARGS__)
#else
#	define TRACE(...) ;
#endif

/*
** PRIVATE DATA TYPES
*/

typedef struct lockinfo
{
	Lock lock;
	Uint32 readerCount;
	Queue *waiting;
	Uint8 hasWriter;
	Uint8 writersWaiting;
} LockInfo;

typedef struct lockwaiter
{
	LockMode mode;
	Pcb *pcb;
} LockWaiter;


/*
** PRIVATE GLOBAL VARIABLES
*/

Uint32 _lock_count = 0;
Queue *_locks;

/*
** PUBLIC GLOBAL VARIABLES
*/

/*
** PRIVATE FUNCTIONS
*/

Status _lock_find_by_key( Lock lock, LockInfo **l ) {
	return _q_get_by_key( _locks, (void **) l, (Key) lock );
}

/*
** PUBLIC FUNCTIONS
*/


/*
** _lock_init()
** 
** Initializes the lock module.
*/
void _lock_init( void ) {
	Status status = _q_alloc( &_locks, _comp_ascend_uint );
	if( status != SUCCESS ) {
		_kpanic( "_lock_init",
			 "_locks alloc status %s",
			 status );
	}
	c_puts( " locks" );
}


/*
** _lock_new()
** 
** Creates a new lock.
*/
Status _lock_new( Lock *lock ) {
	Status status;

	// Initalize the lock info object
	LockInfo *l = (LockInfo*) __kmalloc( sizeof(LockInfo) );
	l->lock = ++_lock_count;
	l->readerCount = 0;
	l->hasWriter = 0;
	l->writersWaiting = 0;
	
	// Create its wating queue
	status = _q_alloc( &l->waiting, _comp_ascend_uint );
	if ( status != SUCCESS) {
		__kfree((void*) l);
		return ( FAILURE );
	}
	*lock = l->lock;

	// Add it to the queue of locks
	_q_insert( _locks, (void*) l, (Key) l->lock );
	if ( status != SUCCESS) {
		_q_dealloc( l->waiting );
		__kfree((void*) l);
		return ( FAILURE );
	}

	TRACE( "CREATED LOCK %d", l->lock );

	return ( SUCCESS );
}

/*
** _lock_destroy( Lock )
** 
** Destroys a lock.
*/
Status _lock_destroy( Lock lock ) {
	LockInfo *l;
	Status status = _lock_find_by_key(lock, &l);
	if ( status == SUCCESS ) {
		return ( FAILURE );
	} else {
		return ( status );
	}
}

/*
** _lock_lock( Lock, Pcb* )
** 
** Locks a lock with the given mode.
*/
Status _lock_lock( Lock lock, LockMode mode, Pcb *pcb ) {
	LockInfo *l;
	Status status = _lock_find_by_key(lock, &l);
	if ( status == SUCCESS ) {
		return ( FAILURE );
	} else {
		return ( status );
	}
}

/*
** _lock_unlock( Lock, LockMode, Pcb* )
** 
** Unlocks a lock with the given mode
*/
Status _lock_unlock( Lock lock, LockMode mode, Pcb *pcb ) {
	LockInfo *l;
	Status status = _lock_find_by_key(lock, &l);
	if ( status == SUCCESS ) {
		return ( SUCCESS );
	} else {
		return ( status );
	}
}

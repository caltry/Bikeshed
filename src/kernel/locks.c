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

#define TRUE 1
#define FALSE 0

/*
** PRIVATE DATA TYPES
*/

typedef struct lockinfo
{
	Lock lock;
	Uint32 readerCount;
	Uint32 writerWaitingCount;
	Queue *waiting;
	Uint8 hasWriter;
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

Status _lock_insert_waiter( LockInfo *l, LockMode mode, Pcb *pcb ) {
	LockWaiter *waiter = (LockWaiter*) __kmalloc( sizeof(LockWaiter) );
	waiter->mode = mode;
	waiter->pcb = pcb;

	Key k;
	if ( mode == LOCK_READ )
	{
		k = (Key) 1;
	} else {
		k = (Key) 0;
	}

	Status status = _q_insert( l->waiting, (void*) waiter, k );
	if ( status == SUCCESS )
	{
		if ( mode == LOCK_WRITE ) {
			l->writerWaitingCount++;
		}
		_dispatch();
	} else {
		__kfree( (void*) waiter );
	}

	return ( status );
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
	l->hasWriter = FALSE;
	l->writerWaitingCount = 0;
	
	// Create its waiting queue
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

	TRACE( "CREATED LOCK %d\n", l->lock );

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
	Status status = ( _lock_find_by_key(lock, &l) );
	if ( status == SUCCESS ) {

		// If we currently have a writer then we should put this pcb into the wait queue and dispatch
		// another process
		if ( l->hasWriter || l->writerWaitingCount > 0 ) {
			TRACE( "LOCK %d ADDED READ WAITER\n", lock );
			return ( _lock_insert_waiter( l, mode, pcb ) );
		} else {
			// if this is requesting read just increment the amount of readers.
			if ( mode == LOCK_READ ) {
				l->readerCount++;
				TRACE( "LOCK %d ADDED READER\n", lock );
				return SUCCESS;

			// if this is requesting write
			} else {
				// if there are still readers
				if ( l->readerCount > 0 ) {
					TRACE( "LOCK %d ADDED WRITE WAITER\n", lock, "WRITE" );
					return ( _lock_insert_waiter( l, mode, pcb ) );

				// no readers. this process is gonna run the show
				} else {
					l->hasWriter = TRUE;
					TRACE( "LOCK %d LOCKED(%s)\n", lock, "WRITE" );
					return ( SUCCESS );
				}
			}
		}
	} else {
		return ( status );
	}
}

/*
** _lock_unlock( Lock, LockMode, Pcb* )
** 
** Unlocks a lock with the given mode
*/
Status _lock_unlock( Lock lock, LockMode mode ) {
	LockInfo *l;
	Status status = _lock_find_by_key(lock, &l);
	if ( status == SUCCESS ) {

		// if this is a reader unlock
		if ( mode == LOCK_READ ) {
			l->readerCount--;
		}

		// check if anything is waiting and make it active
		if ( l->writerWaitingCount > 0 || l->hasWriter ) {
			//pop it
			LockWaiter *waiter;
			status = _q_remove( l->waiting, (void**) &waiter );
			if ( status == SUCCESS ) {
				// schedule it to run
				_sched( waiter->pcb );

				// if it is waiting for read then we need to flush all the rest of the reads
				if ( waiter->mode == LOCK_READ ) {
					l->readerCount++;
					l->hasWriter = FALSE;

					// flush the rest of the readers from the wait queue
					LockWaiter *readWaiter;
					while( status == SUCCESS ) {
						status = _q_remove( l->waiting, (void**) &readWaiter );
						if ( status == SUCCESS ) {
							l->readerCount++;
							_sched( readWaiter->pcb );
							__kfree( (void*) readWaiter );
						}
					}

				// if its waiting to write
				} else {
					l->writerWaitingCount--;
					l->hasWriter = TRUE;
				}


				__kfree( (void*) waiter );
			} else if ( status == EMPTY_QUEUE && mode == LOCK_WRITE) {
				l->hasWriter = FALSE;
				return ( SUCCESS );
			} else {
				return ( status );
			}
		}

		TRACE( "LOCK %d UNLOCKED(%s)\n", lock, ((mode == LOCK_READ) ? "READ" : "WRITE") );
		return ( SUCCESS );
	} else {
		return ( status );
	}
}

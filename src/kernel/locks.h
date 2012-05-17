/*
** File:	locks.h
**
** Author:	Chris Hossenlopp <crh5026@rit.edu>
**
** Contributor:
**
** Description:	Locks module definitions.
*/

#ifndef _LOCKS_H
#define _LOCKS_H

#include "headers.h"

/*
** General (C and/or assembly) definitions
*/

#ifndef __ASM__20113__

#include "pcbs.h"
#include "queues.h"



/*
** Prototypes
*/

/*
** _lock_init()
** 
** Initializes the lock module.
*/
void _lock_init( void );


/*
** _lock_init()
** 
** Creates a new lock.
*/
Status _lock_new( Lock* lock );

/*
** _lock_destroy( Lock )
** 
** Destroys a lock.
*/
Status _lock_destroy( Lock lock );

/*
** _lock_lock( Lock, Pcb* )
** 
** Locks a lock with the given mode.
*/
Status _lock_lock( Lock lock, LockMode mode, Pcb *pcb );

/*
** _lock_unlock( Lock, LockMode )
** 
** Unlocks a lock with the given mode
*/
Status _lock_unlock( Lock lock, LockMode mode );

#endif

#endif
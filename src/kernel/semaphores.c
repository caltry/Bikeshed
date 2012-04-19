/*
** File:	semaphores.c
**
** Author:	Chris Hossenlopp <crh5026@rit.edu>
**
** Contributor:
**
** Description:	Semaphore module
*/

#define	__KERNEL__20113__

#include "headers.h"
#include "semaphores.h"
#include "scheduler.h"
#include "queues.h"

/*
** PRIVATE DEFINITIONS
*/

#define MAX_SEMAPHORES 10

/*
** PRIVATE DATA TYPES
*/

/*
** PRIVATE GLOBAL VARIABLES
*/

Uint32 _sem_count = 0;
Queue *_semaphores;

/*
** PUBLIC GLOBAL VARIABLES
*/



/*
** PRIVATE FUNCTIONS
*/

/*
** PUBLIC FUNCTIONS
*/

void _sem_init( void ) {
	Status status = _q_alloc( &_semaphores, _comp_ascend_uint );
	if( status != SUCCESS ) {
		_kpanic( "_sem_init",
			 "_semaphores alloc status %s",
			 status );
	}
}

/*
** _sem_init()
**
** initialize a semaphore
*/

Sem _sem_new( void ) {
	return 0;
}

/*
** _sem_destroy(Sem*)
**
** schedule a process for execution according to is priority
**
** returns the status of the insertion into the ready queue
*/

void _sem_destroy( Sem sem ) {

}

Uint32 _sem_get_value( Sem sem ) {
	return 0;
}

void _sem_post( Sem sem ) {

}

void _sem_decrement( Sem sem ) {

}
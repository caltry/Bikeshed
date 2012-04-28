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
#include "klibc.c"


//#define TRACE_SEMAPHORES
#ifdef TRACE_SEMAPHORES
#	define TRACE(...) c_printf(__VA_ARGS__)
#else
#	define TRACE(...) ;
#endif


/*
** PRIVATE DEFINITIONS
*/



/*
** PRIVATE DATA TYPES
*/

typedef struct semaphore
{
	Sem sem;
	Uint32 value;
	Queue *waiting;
} Semaphore;

/*
** PRIVATE GLOBAL VARIABLES
*/

Uint32 _sem_count = 0;
Semaphore _sems[MAX_SEMAPHORES];
Queue *_semaphores;
Queue *_available_semaphores;


/*
** PUBLIC GLOBAL VARIABLES
*/

/*
** PRIVATE FUNCTIONS
*/

void _sem_sched_waiting( Semaphore *s ) {
	Status status = SUCCESS;
	while( s->value > 0 && status == SUCCESS) {
		Pcb *pcb;
		status = _q_remove( s->waiting, (void **) &pcb );
		if ( status == SUCCESS ) {
			_sched(pcb);
			s->value--;
		}
	}
}

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

	status = _q_alloc( &_available_semaphores, NULL );
	if( status != SUCCESS ) {
		_kpanic( "_sem_init",
			 "_available_semaphores alloc status %s",
			 status );
	}

	int i;
	for(i = 0; i < MAX_SEMAPHORES; i++) {
		_q_insert(_available_semaphores, &_sems[i], (Key) i);
	}

	TRACE( " semaphores" );
}

/*
** _sem_new()
**
** initialize a semaphore
*/

Sem _sem_new( void ) {
	Semaphore *s;
	Status status =	_q_remove(_available_semaphores, (void **) &s);
	if (status == SUCCESS) {
		s->sem = ++_sem_count;
		s->value = 0;
		_q_alloc(&(s->waiting), NULL);
		_q_insert(_semaphores, (void *) s, (Key) s->sem);
		TRACE("CREATED SEMAPHORE %d\n", s->sem);
		return s->sem;
	} else {
		return -1;
	}
}

/*
** _sem_destroy(Sem*)
**
** schedule a process for execution according to is priority
**
** returns the status of the insertion into the ready queue
*/

Status _sem_destroy( Sem sem ) {
	Semaphore *s;
	Status status = _q_get_by_key(_semaphores, (void **) &s, (Key) sem);

	if(status == SUCCESS) {
		//Put all the waiting processes back into the wait queue with an error
		while(status == SUCCESS) {
			Pcb *pcb;
			status = _q_remove( s->waiting, (void **) &pcb );
			if ( status == SUCCESS ) {
				RET(pcb) = FAILURE;
				_sched(pcb);
			}
		}

		//dealloc its wait queue
		_q_dealloc( s->waiting );

		//put this semaphore back on the queue to be used
		_q_remove_by_key(_semaphores, (void **) &s, (Key) sem);
		_q_insert(_available_semaphores, (void *) s, (Key) 0);

		TRACE( "DESTROYED SEMAPHORE %d\n", sem);
		return SUCCESS;
	} else {
		return status;
	}
}

Status _sem_try_wait( Sem sem ) {
	Semaphore *s;
	Status status = _q_get_by_key(_semaphores, (void **) &s, (Key) sem);
	if(status == SUCCESS) {
		if( s->value > 0 ) {
			s->value--;
			return SUCCESS;
		} else {
			return FAILURE;
		}
	} else {
		return status;
	}

}

Status _sem_post( Sem sem ) {
	Semaphore *s;
	Status status = _q_get_by_key(_semaphores, (void **) &s, (Key) sem);

	//if we found the semaphore
	if (status == SUCCESS)
	{
		s->value++;
		_sem_sched_waiting(s);
		return SUCCESS;
	} else {
		TRACE("herp\n");
		return status;
	}
}

Status _sem_wait( Sem sem, Pcb *pcb ) {
	Semaphore *s;
	Status status = _q_get_by_key(_semaphores, (void **) &s, (Key) sem);

	//if we found the semaphore
	if (status == SUCCESS)
	{
		//if its value is greater than 0 just decrement it and go
		if (s->value > 0) {
			s->value--;
		} else {
			//add this pcb to the waiting queue
			_q_insert(s->waiting, (void *) pcb, (Key) 0);
			_dispatch();
		}
		return SUCCESS;
	} else {
		return status;
	}
}
/*
** File:	semaphores.h
**
** Author:	Chris Hossenlopp <crh5026@rit.edu>
**
** Contributor:
**
** Description:	Semaphore module definitions.
*/

#ifndef _SEMAPHORES_H
#define _SEMAPHORES_H

#include "headers.h"

/*
** General (C and/or assembly) definitions
*/

#ifndef __ASM__20113__

/*
** Start of C-only definitions
*/

#include "pcbs.h"
#include "queues.h"


#define MAX_SEMAPHORES 10

/*
** Types
*/




/*
** Globals
*/


/*
** Prototypes
*/

void _sem_init( void );

/*
** _sem_init()
**
** initialize a semaphore
*/

Sem _sem_new( void );

/*
** _sem_destroy(Sem*)
**
** schedule a process for execution according to is priority
**
** returns the status of the insertion into the ready queue
*/

void _sem_destroy( Sem sem );

Uint32 _sem_get_value( Sem sem );

Status _sem_post( Sem sem );

Status _sem_wait( Sem sem, Pcb *pcb );

#endif



#endif
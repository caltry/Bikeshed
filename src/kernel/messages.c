/*
** File:	messages.c
**
** Author:	Chris Hossenlopp <crh5026@rit.edu>
**
** Contributor:
**
** Description:	Message Passing module.
*/

#define	__KERNEL__20113__

#include "defs.h"
#include "types.h"
#include "messages.h"
#include "scheduler.h"
#include "queues.h"
#include "klib.h"
#include "c_io.h"
#include "memory/kmalloc.h"

#define TRACE_MESSAGES
#ifdef TRACE_MESSAGES
#	define TRACE(...) c_printf(__VA_ARGS__)
#else
#	define TRACE(...) ;
#endif

#define TRUE 1
#define FALSE 0

/*
** PRIVATE DATA TYPES
*/



/*
** PRIVATE GLOBAL VARIABLES
*/

Queue *_messageQueues;
Queue *_waitingPcbs;

/*
** PUBLIC GLOBAL VARIABLES
*/

/*
** PRIVATE FUNCTIONS
*/


/*
** PUBLIC FUNCTIONS
*/


/*
** _messages_init()
** 
** Initializes the message passing module.
*/
void _messages_init( void ) {
	Status status = _q_alloc( &_messageQueues, _comp_ascend_uint );
	if( status != SUCCESS ) {
		_kpanic( "_messages_init",
			 "_messageQueues alloc status %s",
			 status );
	}
	status = _q_alloc( &_waitingPcbs, _comp_ascend_uint );
	if( status != SUCCESS ) {
		_kpanic( "_messages_init",
			 "_waitingPcbs alloc status %s",
			 status );
	}
	c_puts( " messages" );
}


/*
** _message_kern_send( Pid, void*, UInt32 )
** 
** Sends a message to a process coming from the kernel. This disables interrupts before it does the call
*/
Status _message_kern_send( Pid toPid, void *data, Uint32 size ) {
	asm volatile ("cli");
	Status status = _message_send( NULL, toPid, data, size );
	asm volatile ("sti");
	return status;
}

/*
** _message_send( Pid, void*, UInt32 )
** 
** Sends a message to a process coming from the kernel. Use this call when interrupts are already disabled.
*/
Status _message_ikern_send( Pid toPid, void *data, Uint32 size ) {
	return _message_send( NULL, toPid, data, size );
}

/*
** _message_send( Pid, Pid, void*, UInt32 )
** 
** Sends a message to a process.
*/
Status _message_send( Pid fromPid, Pid toPid, void *data, Uint32 size ) {
	return ( FEATURE_UNIMPLEMENTED );
}

/*
** _message_receive( Pcb, Pid, Pid*, void**, UInt32* )
** 
** Receives a message.
*/
Status _message_receive( Pcb pcb, Pid toPid, Pid *fromPid, void **data, Uint32 *size ){
	return ( FEATURE_UNIMPLEMENTED );
}

/*
** _message_try_receive( Pid, Pid*, void**, UInt32* )
** 
** Receives a message if there is one.
*/
Status _message_try_receive( Pid toPid, Pid *fromPid, void **data, Uint32 *size ) {
	return ( FEATURE_UNIMPLEMENTED );
}

/*
** _message_has_message( Pid )
** 
** Checks if there is a message waiting.
*/
Status _message_has_message( Pid pid ) {
	return ( FEATURE_UNIMPLEMENTED );
}

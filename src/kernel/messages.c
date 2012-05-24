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
#include "memory/paging.h"
#include "memory/physical.h"

//#define TRACE_MESSAGES
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

typedef struct message
{
	void* data;
	Uint32 size;
	Pid fromPid;
} Message;


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
// CLOSENOUGH MEMORY ALLOCATION FOR MESSAGES THIS IS BAD AND I SHOULD FEEL BAD.
#define MESSAGE_LOCATION_START 0xB0000000
void *_messageLoc = MESSAGE_LOCATION_START;

void *_messages_copy_kern_to_proc( Pcb *toPcb, void *data, Uint32 size ) {
	__virt_switch_page_directory( toPcb->page_directory );
	__virt_map_page( __phys_get_free_4k(), _messageLoc, PG_USER | PG_READ_WRITE | PG_PRESENT );
	void *ptr = _messageLoc;
	_kmemcpy( ptr, data, size );
	_messageLoc += PAGE_SIZE;
	
	__virt_switch_page_directory( _current->page_directory );
	return ptr;
}

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
	TRACE("MESSAGES: Message from %d to %d \"%s\"\n", fromPid, toPid, data);

	//first check to see if the to process is wating for a message.
	Pcb *pcb;
	Status status = _q_remove_by_key( _waitingPcbs, (void**) &pcb, (Key) (toPid * 1) );
	if ( status == SUCCESS ) {
		//copy into kernel memory
		void *dataCopy = __kmalloc( size );
		_kmemcpy( dataCopy, data, size );
		void *finalCopy = _messages_copy_kern_to_proc( pcb, dataCopy, size );

		__virt_switch_page_directory( pcb->page_directory );
		//We found it waiting set all the things and schedule it to run
		*((Pid*) ARG(pcb)[1]) = fromPid;
		*((void**) ARG(pcb)[2]) = finalCopy;
		*((Uint32*) ARG(pcb)[3]) = size;
		RET(pcb) = SUCCESS;
		__virt_switch_page_directory( _current->page_directory );

		_sched(pcb);

		return ( SUCCESS );
	}


	//find queue for toPid
	Queue *msgQueue;
	status = _q_get_by_key( _messageQueues, (void **) &msgQueue, (Key) (toPid * 1) );
	if ( status == NOT_FOUND || status == EMPTY_QUEUE ) { //make the queue if it doesnt exist
		//Make a queue
		status = _q_alloc( &msgQueue, NULL );
		if ( status != SUCCESS ) {
			return ( FAILURE );
		}
		_q_insert( _messageQueues, (void*) msgQueue, (Key) (toPid * 1) );
		TRACE( "MESSAGES made queue for %d.\n", toPid );
	}

	//create struct and copy data
	Message *msg = __kmalloc( sizeof(Message) );
	msg->data = __kmalloc( size );
	msg->size = size;
	msg->fromPid = fromPid;
	_kmemcpy( msg->data, data, size );
	TRACE("MESSAGES: Message put in queue from %d \"%s\"\n",  msg->fromPid, msg->data);

	//insert the message
	_q_insert( msgQueue, (void*) msg, (Key) 0 );
}

/*
** _message_receive( Pcb, Pid, Pid*, void**, UInt32* )
** 
** Receives a message.
*/
Status _message_receive( Pcb *pcb, Pid toPid, Pid *fromPid, void **data, Uint32 *size ){
	// find the queue for this process if one exists
	Queue *msgQueue;
	Status status = _q_get_by_key( _messageQueues, (void **) &msgQueue, (Key) (toPid * 1) );
	if ( status == SUCCESS ) { //queue found
		Message *msg;
		status = _q_remove( msgQueue, (void**) &msg );
		if ( status == SUCCESS ) {
			*fromPid = msg->fromPid;
			*data = _messages_copy_kern_to_proc( pcb, msg->data, msg->size );
			*size = msg->size;
			TRACE("MESSAGES: Message received from %d to %d \"%s\"\n",  msg->fromPid, toPid, msg->data);
			return ( SUCCESS );
		}
	}
	// we must have to wait now since there is no queue or message waiting
	_q_insert( _waitingPcbs, (void*) pcb, (Key) (toPid * 1) );
	_dispatch();
	return ( FAILURE );
}

/*
** _message_try_receive( Pid, Pid*, void**, UInt32* )
** 
** Receives a message if there is one.
*/
Status _message_try_receive( Pid toPid, Pid *fromPid, void **data, Uint32 *size ) {
	// find the queue for this process if one exists
	Queue *msgQueue;
	Status status = _q_get_by_key( _messageQueues, (void **) &msgQueue, (Key) (toPid * 1) );
	if ( status == SUCCESS ) { //queue found
		Message *msg;
		status = _q_remove( msgQueue, (void**) &msg );
		if ( status == SUCCESS ) {
			*fromPid = msg->fromPid;
			*data = _messages_copy_kern_to_proc( _current, msg->data, msg->size );
			*size = msg->size;
			TRACE("MESSAGES: Message received from %d to %d \"%s\"\n",  msg->fromPid, toPid, msg->data);
			return ( SUCCESS );
		}
	}

	return ( NOT_FOUND );
}

/*
** _message_has_message( Pid )
** 
** Checks if there is a message waiting.
*/
Status _message_has_message( Pid toPid ) {
	// find the queue for this process if one exists
	Queue *msgQueue;
	Status status = _q_get_by_key( _messageQueues, (void **) &msgQueue, (Key) (toPid * 1) );
	if ( status == SUCCESS ) { //queue found
		Message *msg;
		if ( ! _q_empty( msgQueue ) ) {
			return ( SUCCESS );
		}
	}

	return ( NOT_FOUND );
}

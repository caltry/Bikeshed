/*
** File:	messages.h
**
** Author:	Chris Hossenlopp <crh5026@rit.edu>
**
** Contributor:
**
** Description:	Message passing module definitions.
*/

#ifndef _LOCKS_H
#define _LOCKS_H

#include "types.h"

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
** _messages_init()
** 
** Initializes the mesage passing module.
*/
void _messages_init( void );


/*
** _message_kern_send( Pid, void*, UInt32 )
** 
** Sends a message to a process coming from the kernel. This disables interrupts before it does the call
*/
Status _message_kern_send( Pid toPid, void *data, Uint32 size );

/*
** _message_send( Pid, void*, UInt32 )
** 
** Sends a message to a process coming from the kernel. Use this call when interrupts are already disabled.
*/
Status _message_ikern_send( Pid toPid, void *data, Uint32 size );

/*
** _message_send( Pid, Pid, void*, UInt32 )
** 
** Sends a message to a process.
*/
Status _message_send( Pid fromPid, Pid toPid, void *data, Uint32 size );

/*
** _message_receive( Pcb, Pid, Pid*, void**, UInt32* )
** 
** Receives a message.
*/
Status _message_receive( Pcb pcb, Pid toPid, Pid *fromPid, void **data, Uint32 *size );

/*
** _message_try_receive( Pid, Pid*, void**, UInt32* )
** 
** Receives a message if there is one.
*/
Status _message_try_receive( Pid toPid, Pid *fromPid, void **data, Uint32 *size );

/*
** _message_has_message( Pid )
** 
** Checks if there is a message waiting.
*/
Status _message_has_message( Pid pid );

#endif

#endif

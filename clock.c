/*
** SCCS ID:	@(#)clock.c	1.1	4/5/12
**
** File:	clock.c
**
** Author:	4003-506 class of 20113
**
** Contributor:
**
** Description:	Clock module
*/

#define	__KERNEL__20113__

#include "headers.h"

#include <x86arch.h>
#include "startup.h"

#include "clock.h"
#include "pcbs.h"
#include "queues.h"
#include "scheduler.h"
#include "sio.h"
#include "syscalls.h"

/*
** PRIVATE DEFINITIONS
*/

/*
** PRIVATE DATA TYPES
*/

/*
** PRIVATE GLOBAL VARIABLES
*/

// pinwheel control variables

static Uint32 _pinwheel;
static Uint32 _pindex;

/*
** PUBLIC GLOBAL VARIABLES
*/

// the system time

Time _system_time;

/*
** PRIVATE FUNCTIONS
*/

/*
** PUBLIC FUNCTIONS
*/

/*
** _clock_init()
**
** initialize the clock module
*/

void _clock_init( void ) {
	Uint32 divisor;

	// start the pinwheel

	_pinwheel = _pindex = 0;

	// return to the epoch

	_system_time = 0;

	// set the clock to tick at CLOCK_FREQUENCY Hz.

	divisor = TIMER_FREQUENCY / CLOCK_FREQUENCY;
	__outb( TIMER_CONTROL_PORT, TIMER_0_LOAD | TIMER_0_SQUARE );
        __outb( TIMER_0_PORT, divisor & 0xff );        // LSB of divisor
        __outb( TIMER_0_PORT, (divisor >> 8) & 0xff ); // MSB of divisor

	// register the ISR

	__install_isr( INT_VEC_TIMER, _isr_clock );

        // announce that we have initialized the clock module

        c_puts( " clock" );
}


/*
** _isr_clock(vector,code)
**
** clock ISR
*/

void _isr_clock( int vector, int code ) {
	Pcb *pcb;
	Key key;
	Status status;

	// spin the pinwheel

	++_pinwheel;
	if( _pinwheel == (CLOCK_FREQUENCY / 10) ) {
		_pinwheel = 0;
		++_pindex;
		c_putchar_at( 79, 0, "|/-\\"[ _pindex & 3 ] );
	}

	// increment the system time

	++_system_time;

	// wake up any sleeper whose time has come
	//
	// we give awakened processes preference over the
	// current process (when it is scheduled again)

	do {
		// check the wakeup time of the first process

		status = _q_peek( _sleeping, &key );
		if( status == EMPTY_QUEUE ) {
			break;
		}

		// if we haven't reached that time yet, no more to awaken
		if( key.u > _system_time ) {
			break;
		}

		// wake up the first sleeper
		status = _q_remove( _sleeping, (void **) &pcb );
		if( status != SUCCESS ) {
			_kpanic( "_isr_clock",
				"sleepq remove status %s",
				status );
		}

		// and schedule it
		status = _sched( pcb );
		if( status != SUCCESS ) {
			c_printf( "clock: kill sleeper %d\n", pcb->pid );
			_cleanup( pcb );
		}
	} while( 1 );

	// Approximately every 20 seconds, dump the queues, and
	// print the contents of the SIO buffers.

	if( (_system_time % 10000) == 0 ) {
		c_printf( "Queue contents @%08x\n", _system_time );
		_q_dump( "ready[0]", _ready[0] );
		_q_dump( "ready[1]", _ready[1] );
		_q_dump( "ready[2]", _ready[2] );
		_q_dump( "ready[3]", _ready[3] );
		_q_dump( "sleep", _sleeping );
		_q_dump( "read", _reading );
		_sio_dump();
	}

	// if the current process has used its quantum, reschedule it
	// and dispatch another one

	if( _current->quantum < 1 ) {
		status = _sched( _current );
		if( status != SUCCESS ) {
			c_printf( "clock: kill current %d\n", _current->pid );
			_cleanup( _current );
		}
		_dispatch();
	} else {
		(_current->quantum)--;
	}

	// tell the PIC we're done

	__outb( PIC_MASTER_CMD_PORT, PIC_EOI );

}

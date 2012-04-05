/*
** SCCS ID:	@(#)system.c	1.1	4/5/12
**
** File:	system.c
**
** Author:	4003-506 class of 20113
**
** Contributor:
**
** Description:	Miscellaneous OS support functions
*/

#define	__KERNEL__20113__

#include "headers.h"

#include "system.h"
#include "clock.h"
#include "pcbs.h"
#include "bootstrap.h"
#include "syscalls.h"
#include "sio.h"
#include "scheduler.h"

// need init() address
#include "users.h"

// need the exit() prototype
#include "ulib.h"

/*
** PUBLIC FUNCTIONS
*/

/*
** _put_char_or_code( ch )
**
** prints the character on the console, unless it is a non-printing
** character, in which case its hex code is printed
*/

void _put_char_or_code( int ch ) {

	if( ch >= ' ' && ch < 0x7f ) {
		c_putchar( ch );
	} else {
		c_printf( "\\x%02x", ch );
	}

}

/*
** _cleanup(pcb)
**
** deallocate a pcb and associated data structures
*/

void _cleanup( Pcb *pcb ) {
	Status status;

	if( pcb == NULL ) {
		return;
	}

	if( pcb->stack != NULL ) {
		status = _stack_dealloc( pcb->stack );
		if( status != SUCCESS ) {
			_kpanic( "_cleanup", "stack dealloc status %s\n", status );
		}
	}

	pcb->state = FREE;
	status = _pcb_dealloc( pcb );
	if( status != SUCCESS ) {
		_kpanic( "_cleanup", "pcb dealloc status %s\n", status );
	}

}


/*
** _create_process(pcb,entry)
**
** initialize a new process' data structures (PCB, stack)
**
** returns:
**	success of the operation
*/

Status _create_process( Pcb *pcb, Uint32 entry ) {
	Context *context;
	Stack *stack;
	Uint32 *ptr;

	// don't need to do this if called from _sys_exec(), but
	// we are called from other places, so...

	if( pcb == NULL ) {
		return( BAD_PARAM );
	}

	// if the PCB doesn't already have a stack, we
	// need to allocate one

	stack = pcb->stack;
	if( stack == NULL ) {
		stack = _stack_alloc();
		if( stack == NULL ) {
			return( ALLOC_FAILED );
		}
		pcb->stack = stack;
	}

	// clear the stack

	_kmemclr( (void *) stack, sizeof(Stack) );

	/*
	** Set up the initial stack contents for a (new) user process.
	**
	** We reserve one longword at the bottom of the stack as
	** scratch space.  Above that, we simulate a call to exit() by
	** pushing the address of exit() as a "return address".  Finally,
	** above that we place an context_t area that is initialized with
	** the standard initial register contents.
	**
	** The low end of the stack will contain these values:
	**
	**      esp ->  ?       <- context save area
	**              ...     <- context save area
	**              ?       <- context save area
	**              exit    <- return address for main()
	**              filler  <- last word in stack
	**
	** When this process is dispatched, the context restore
	** code will pop all the saved context information off
	** the stack, leaving the "return address" on the stack
	** as if the main() for the process had been "called" from
	** the exit() stub.
	*/

	// first, compute a pointer to the second-to-last longword

	ptr = ((Uint32 *) (stack + 1)) - 2;

	// assign the "return" address

	*ptr = (Uint32) exit;

	// next, set up the process context

	context = ((Context *) ptr) - 1;
	pcb->context = context;

	// initialize all the fields that should be non-zero, starting
	// with the segment registers

	context->cs = GDT_CODE;
	context->ss = GDT_STACK;
	context->ds = GDT_DATA;
	context->es = GDT_DATA;
	context->fs = GDT_DATA;
	context->gs = GDT_DATA;

	// EFLAGS must be set up to re-enable IF when we switch
	// "back" to this context

	context->eflags = DEFAULT_EFLAGS;

	// EIP must contain the entry point of the process; in
	// essence, we're pretending that this is where we were
	// executing when the interrupt arrived

	context->eip = entry;

	return( SUCCESS );

}


/*
** _init - system initialization routine
**
** Called by the startup code immediately before returning into the
** first user process.
*/

void _init( void ) {
	Pcb *pcb;
	Status status;

	/*
	** BOILERPLATE CODE - taken from basic framework
	**
	** Initialize interrupt stuff.
	*/

	__init_interrupts();	// IDT and PIC initialization

	/*
	** Console I/O system.
	*/

	c_io_init();
	c_setscroll( 0, 7, 99, 99 );
	c_puts_at( 0, 6, "================================================================================" );

	/*
	** 20113-SPECIFIC CODE STARTS HERE
	*/

	/*
	** Initialize various OS modules
	*/

	c_puts( "Module init: " );

	_q_init();		// must be first
	_pcb_init();
	_stack_init();
	_sio_init();
	_syscall_init();
	_sched_init();
	_clock_init();

	c_puts( "\n" );

	/*
	** Create the initial system ESP
	**
	** This will be the address of the next-to-last
	** longword in the system stack.
	*/

	_system_esp = ((Uint32 *) ( (&_system_stack) + 1)) - 2;

	/*
	** Install the ISRs
	*/

	__install_isr( INT_VEC_TIMER, _isr_clock );
	__install_isr( INT_VEC_SYSCALL, _isr_syscall );
	__install_isr( INT_VEC_SERIAL_PORT_1, _isr_sio );

	/*
	** Create the initial process
	**
	** Code mostly stolen from _sys_fork(); if that routine
	** changes, SO MUST THIS!!!
	**
	** First, get a PCB and a stack
	*/

	pcb = _pcb_alloc();
	if( pcb == NULL  ) {
		_kpanic( "_init", "first pcb alloc failed\n", FAILURE );
	}

	pcb->stack = _stack_alloc();
	if( pcb->stack == NULL ) {
		_kpanic( "_init", "first stack alloc failed\n", FAILURE );
	}

	/*
	** Next, set up various PCB fields
	*/

	pcb->pid  = _next_pid++;
	pcb->ppid = pcb->pid;
	pcb->priority = PRIO_HIGH;	// init() should finish first

	/*
	** Set up the initial process context.
	*/

	status = _create_process( pcb, (Uint32) init );
	if( status != SUCCESS ) {
		_kpanic( "_init", "create init process status %s\n", status );
	}

	/*
	** Make it the first process
	*/

	_current = pcb;

	/*
	** Turn on the SIO receiver (the transmitter will be turned
	** on/off as characters are being sent)
	*/

	_sio_enable( SIO_RX );

	/*
	** END OF 20113-SPECIFIC CODE
	**
	** Finally, report that we're all done.
	*/

	c_puts( "System initialization complete.\n" );

}

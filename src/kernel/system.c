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

#include "defs.h"
#include "types.h"

#include "system.h"
#include "clock.h"
#include "pcbs.h"
#include "bootstrap.h"
#include "syscalls.h"
#include "sio.h"
#include "scheduler.h"
#include "semaphores.h"
#include "c_io.h"
#include "lib/klib.h"
#include "support.h"
#include "pcbs.h"
#include "scheduler.h"
#include "messages.h"

#include "bootstrap.h"
#include "memory/physical.h"
#include "memory/paging.h"
#include "memory/kmalloc.h"
#include "network/e100.h"
#include "pci/pci.h"
#include "serial.h"
#include "fs/ext2/ext2.h"
#include "vfs_init.h"
#include "cpp/cppinit.h"
#include "cpp/cpptest.hpp"
#include "elf/elf.h"
#include "locks.h"
#include "vm8086/vm8086.h"

#include "bios/bios.h"

#include "video/video.h"
#include "video/desktop.h"

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

	pcb->state = FREE;
	status = _pcb_dealloc( pcb );
	if( status != SUCCESS ) {
		_kpanic( "_cleanup", "pcb dealloc status %s\n", status );
	}

	if (pcb != _current)
	{
		serial_printf("PCB isn't current!\n");
		__virt_switch_page_directory(pcb->page_directory);
		__virt_dealloc_page_directory();
		__virt_switch_page_directory(_current->page_directory);
	} else {
		serial_printf("PCB is current!\n");
		__virt_dealloc_page_directory(); 
		// Just in case, dispatch() will change it again
		__virt_switch_page_directory(__virt_kpage_directory);
	}
}

void _kill_thread( Pcb* pcb )
{
	if ( pcb == NULL ) {
		return;
	}

	Status status;	
	if ( pcb->stack != NULL ) {
		status = _stack_dealloc( pcb->stack );
		if ( status != SUCCESS ) {
			_kpanic( "kill thread", "stack dealloc status %s\n", status );
		}
	}

	pcb->state = FREE;
	status = _pcb_dealloc( pcb );
	if ( status != SUCCESS ) {
		_kpanic("_kill_thread", "pcb kill thread status: %s\n", status );
	}

	_dispatch();
}

// A hack for now
Status _create_kernel_thread( const Pcb const* parent, Uint32 entry ) {

	if ( parent == NULL ) {
		return BAD_PARAM;
	}

	Pcb *new = _pcb_alloc();
	if ( new == NULL) {
		_kpanic("_create_kernel_thread", "Failed to create a new thread\n", 0);
		return FAILURE;
	}

	_kmemcpy((void *)new, (void *)parent, sizeof(Pcb));

	new->page_directory = parent->page_directory;

	Stack* stack = new->stack;
	if ( stack == NULL ) {
		stack = _stack_alloc();
		if ( stack == NULL ) {
			return ( ALLOC_FAILED );
		}
		new->stack = stack;
	}

	new->pid  = _next_pid++;
	new->ppid = parent->pid;
	new->state = NEW;

	_kmemclr( (void *)stack, sizeof(Stack));

	Uint32* ptr = ((Uint32 *) (stack + 1)) - 2;
	*ptr = (Uint32) _kill_thread;	

	Context* context = ((Context *) ptr) - 1;
	new->context = context;

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
	
	context->esp = (Uint32)(((Uint32 *)context)) - 4;
	context->ebp = ((Uint32 *)((stack) + 1)) - 2;

	// EIP must contain the entry point of the process; in
	// essence, we're pretending that this is where we were
	// executing when the interrupt arrived

	context->eip = entry;


	Status status = _sched( new );
	if ( status != SUCCESS ) {
		_kill_thread( new );
		return FAILURE;
	}

	serial_printf("Scheduled new thread!\n");
	return( SUCCESS );
}

/*
** _init - system initialization routine
**
** Called by the startup code immediately before returning into the
** first user process.
*/

void _init( void ) {

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

	/* Setup virtual memory
	 */
	c_printf("System end: %x\n", (Uint32)&KERNEL_END);
	c_printf("Memory 1M-16M: %x\n", *((Uint16*)(MMAP_ADDRESS + MMAP_EXT_LO)));
	c_printf("Memory > 16M 64k blocks: %x\n", *((Uint16*)(MMAP_ADDRESS + MMAP_EXT_HI)));
	c_printf("CFG Memory 1M-16M: %x\n", *((Uint16*)(MMAP_ADDRESS + MMAP_CFG_LO)));
	c_printf("CFG Memory > 16M 64k blocks: %x\n", *((Uint16*)(MMAP_ADDRESS + MMAP_CFG_HI)));
	serial_install();
	__phys_initialize_bitmap();
	__virt_initialize_paging();
	__kmem_init_kmalloc();

	_sio_init();
	// Initialize C++ support, we do this after the memory is intialized so static/global
	// C++ objects can use the new operator in their constructors
	__cpp_init();

	__pci_init();
	__pci_dump_all_devices();
//	__net_init();
	__vm8086_init();

	// Test the C++ support
	_test_cpp();

	_q_init();		// must be first
	_pcb_init();
	_stack_init();
	_syscall_init();
	_sched_init();
	_sem_init();
	_lock_init();
	_messages_init();
	_clock_init();
	_init_all_ramdisks();
	_fs_ext2_init();

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
	** Initialize the bios module
	*/

	_bios_init();

	/*
	** Create the initial process
	**
	** Code mostly stolen from _sys_fork(); if that routine
	** changes, SO MUST THIS!!!
	**
	** First, get a PCB and a stack
	*/

	Pcb *pcb = _pcb_alloc();
	if( pcb == NULL  ) {
		_kpanic( "_init", "first pcb alloc failed\n", FAILURE );
	}

	/*
	** Next, set up various PCB fields
	*/

	c_printf("Setting up PCB\n");

	pcb->pid  = _next_pid++;
	pcb->ppid = pcb->pid;
	pcb->priority = PRIO_HIGH;	// init() should finish first
	pcb->page_directory = __virt_clone_directory(); // Clone's the kernels directory
	__virt_switch_page_directory(pcb->page_directory);

	/*
	** Set up the initial process context.
	*/

	Status status;
	char file_path[] = "/etc/initproc";
	if ((status = _elf_load_from_file(pcb, &file_path[0])) != SUCCESS)
	{
		_kpanic("_init", "Failed to load init process: %s\n", status);
	}

	/*
	** Make it the first process
	*/

	_current = pcb;

	/*
	** Start the desktop refreshing thread
	 */
#ifdef VIDEO_ENABLE
	_create_kernel_thread( pcb, (Uint32)_desktop_run );
#endif

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
	serial_string("System intialized\n");
}

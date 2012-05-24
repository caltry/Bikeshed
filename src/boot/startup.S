/*
** SCCS ID:	@(#)startup.S	1.9	10/17/11
**
** File:	startup.S
**
** Author:	Jon Coles
**
** Contributor:	Warren R. Carithers, K. Reek
**
** Description:	SP1/SP2 startup code.
**
** This code prepares the various registers for execution of
** the program.  It sets up all the segment registers and the
** runtime stack.  By the time this code is running, we're in
** protected mode already.
**
** This code is intended to be assembled with as, not as86.
*/

#include "bootstrap.h"

/*
** Configuration options - define in Makefile
**
**	CLEAR_BSS_SEGMENT	include code to clear all BSS space
**	SP2_CONFIG		enable SP2-specific startup variations
*/

/* 
 ** Setup Virtual Memory
 ** 
 ** The boot_page_directory is for switching to our higher 
 ** half kernel. We need an intermediate page directory because
 ** the kernel will be linked at the 3GB line, and so all code
 ** will reference 3GB line addresses. This page directory will
 ** map the 3GB addresses back to the lower memory area so the
 ** kernel code can still work.
 **
 ** To save on space we're using 4MB pages here so we don't need
 ** to specify Page Tables. These 4MB pages will be replaced by
 ** the standard 4KB pages later once the kernel enables virtual
 ** memory for real.
 */
#define KERNEL_VIRTUAL_BASE 0xC0000000					/* Our kernel is loaded at the 3GB line */
#define KERNEL_PAGE_NUMBER (KERNEL_VIRTUAL_BASE >> 22)	/* Page directory in 4MB mode */

#define PAGE_PRESENT 0x1 
#define PAGE_READ_WRITE 0x2
#define PAGE_4MB 0x80

.section .data
.align 4096
.globl boot_page_directory
/* This page directory defines the 4MB page containing the kernel */
boot_page_directory:	/* We use this to identity map the first 1MB */
	.long 0x00000083		/* of memory and the kernel at the 3GB line */
	.fill (KERNEL_PAGE_NUMBER - 1), 4, 0 /* Pages before kernel */
	.long 0x00000083  /* This entry is the kernel in the higher-half */
	.fill (1024 - KERNEL_PAGE_NUMBER - 1), 4, 0 /* Pages after the kernel */

/*
** A symbol for locating the beginning of the code.
*/
	.globl begtext

	.text
begtext:

/*
** The entry point.
*/
	.globl	_start

_start:

/* SETUP VIRTUAL MEMORY FOR THE HIGHER HALF kernel
 * This section of assembly loads the boot_page_directory as 
 * the current page directory and enables paging
 */
	mov $(boot_page_directory - KERNEL_VIRTUAL_BASE), %ecx
	mov %ecx, %cr3	/* Load the Page Directory Base Register */

	mov %cr4, %ecx	/* Set the PSE bit in CR4 to enabled 4MB Pages */
	or $0x00000010, %ecx
	mov %ecx, %cr4

	mov %cr0, %ecx	/* Set PG bit in CR0 to enable paging  */
	or $0x80000000, %ecx
	mov %ecx, %cr0

	/* Start fetching instructions after paging has been enabled */
	lea (enable_paging - KERNEL_VIRTUAL_BASE), %ecx
	jmp *%ecx
enable_paging:
/* Paging is now enabled */

/* Now we need to move to our upper location */
	mov $(move_to_higher), %ecx
	push %ecx
	ret
move_to_higher:


#ifdef CLEAR_BSS_SEGMENT

/*
** Zero the BSS segment
**
** These symbols are defined automatically by the linker.
*/
	.globl	__bss_start,_end

	movl	$__bss_start,%edi
clearbss:
	movl	$0,(%edi)
	addl	$4,%edi
	cmpl	$_end,%edi
	jb	clearbss

#endif
	
/*
** Initialize devices
*/
	mov $_init, %ecx
	call	_init		// initialize the OS
/*
** Restore the first user process context
**
** WARNING:  must set things up so that the stack looks like
** we're returning to the context restore routine after handling
** an interrupt!!!
*/
	jmp	__isr_restore   // defined in isr_stubs.S

/* Call the destructors for the C++ static objects
 *
 * An argument of 0 means call all of the destructors
 */
	push $0
	call __cxa_finalize
/*
** Returned from the main function!  Stop execution.
*/
	cli			// Disable interrupts
	pushl	$message	// Print the message saying that
	pushl	$24		//   the main function returned.
	movl	$80,%eax	//   This goes to the lower right
	subl	$messagelen,%eax //  corner of the screen.
	pushl	%eax
	call	c_puts_at
die:	hlt			// Stop.
	jmp	die

message: .string "     ***** Returned from main! *****     "
messagelen = . - message - 1


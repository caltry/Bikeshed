/*
** File:    bios.S
**
** Author:  Sean Congden
**
** Description: Support functions for making bios calls.
** 
*/

#define __ASM__20113__

#include "bootstrap.h"
#include "vesa.h"


	.text
	.code32
	.globl call_bios
call_bios:
	pushal

	call 	switch_to_real_mode
	.code16

	/* TODO */

	call	switch_to_protected_mode
	.code32

	popal
	ret



/*
** Switches from protected mode to real mode
*/
	.code32
	.globl switch_to_real_mode
switch_to_real_mode:
	cli

	/* Disable paging */
	movl	%cr0, %eax
	andl	$0x7fffffff, %eax
	movl	%eax, %cr0

	/* Save the current ESP and EBP values */
	movl	%esp, %eax
	movl	%eax, RM_SAVED_ESP
	movl	%ebp, RM_SAVED_EBP

	/*
	** Set up the real mode stack
	*/
	
	/* Copy the return address to the real stack */
	movl	(%esp), %ecx
	movw	%cx, (REAL_STACK)

	/* Switch to using the real mode stack */
	movl	$(REAL_STACK), %eax
	movl	%eax, %ebp
	movl	%eax, %esp

	/* Load the real mode GDT selectors */
	movw	$GDT_DATA_16_2, %ax
	movw	%ax, %ds
	movw	%ax, %es
	movw	%ax, %fs
	movw	%ax, %gs
	movw	$GDT_STACK_16_2, %ax
	movw	%ax, %ss

	pushw	%cx

	ljmp	$GDT_CODE_16, $_not_quite_real

_not_quite_real:

	/* Switch of the protected mode flag */
	movl	%cr0, %eax				/* get current CR0 */
	andl	$0xfffffffe, %eax		/* clear the PE bit */
	movl	%eax, %cr0				/* and store it back. */

	/* Long jump to a real mode code segment */
	.byte	0x66
	ljmp	$0x0, $_real_mode_segment

	.code16
_real_mode_segment:

	/* Clear the segment registers */
	xorw	%ax, %ax

	movw	%ax, %ds
	movw	%ax, %es
	movw	%ax, %fs
	movw	%ax, %gs
	movw	%ax, %ss

	/* Load the real mode IDT */
	lidt	(_real_idt)

	sti
	ret



/*
** Returns to protected mode from real mode
*/
	.code16
	.globl switch_to_protected_mode
switch_to_protected_mode:
	/* Load the protected mode IDT */
	lidt	(_idt_48)

	/* Set the protected mode flag */
	movl	%cr0, %eax				/* get current CR0 */
	orb		$0x1, %al				/* set the PE bit */
	movl	%eax, %cr0				/* and store it back. */
	
	/* Long jump to a real mode code segment */
	.byte	0x66
	.code32
	ljmp	$GDT_CODE, $_protected_mode_segment

_protected_mode_segment:

	/* Load the real mode GDT selectors */
	movw	$GDT_DATA, %ax
	movw	%ax, %ds
	movw	%ax, %es
	movw	%ax, %fs
	movw	%ax, %gs
	movw	$GDT_STACK, %ax
	movw	%ax, %ss

	/* Save the current return address */
	xorl %eax, %eax
	movw %sp, %ax
	movl (%eax), %ebx /* EAX holds the return address */
	andl $0x0000FFFF, %ebx
	
	/* Switch to the protected mode stack */
	movl RM_SAVED_ESP, %eax /* restore saved ESP and EBP */
	movl %eax, %esp
	movl RM_SAVED_EBP, %ebp

	/* Enable paging */
	movl	%cr0, %eax
	orl		$0x80000000, %eax
	movl	%eax, %cr0

	movl	%ebx, (%esp)

	sti
	ret



	.code16
_real_idt:
	.short 0x3ff
	.long 0x0

_idt_48:
	.word	0x0800
	.quad	IDT_ADDRESS

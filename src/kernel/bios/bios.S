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
	.globl vesa_test
vesa_test:
/*	pushal

	call switch_to_real_mode
	.code16

	movw $mymsg, %si
	call dispMsg

	call switch_to_protected_mode
	.code32*/

	movl	$0xc001c0de, %eax
	cli
	hlt

;	popal
	ret



	.code16
dispMsg:	
	pushw	%ax
	pushw	%bx
repeat:
	lodsb			/* grab next character */

	movb	$0x0e, %ah	/* write and advance cursor */
	movw	$0x07, %bx	/* page 0, white on blank, no blink */
	orb	%al, %al	/* AL is character to write */
	jz	getOut		/* if we've reached the NUL, get out */

	int	$0x10		/* otherwise, print and repeat */
	jmp	repeat	

getOut:				/* we're done, so return */
	popw	%bx
	popw	%ax
	ret


mymsg:
	.asciz "\n\rBIOS CALL TEST\n\r"

/*
** Switches from protected mode to real mode
*/
	.code32
	.globl switch_to_real_mode
switch_to_real_mode:
	pushal

	cli

	/* Save the current ESP and EBP values */
	movl	%esp, %eax
	movl	%eax, RM_SAVED_ESP
	movl	%ebp, RM_SAVED_EBP

	/* Set up the real mode stack */
	movl	(%esp), %eax		/* copy the return address to the real stack */
	movw	%ax, (REAL_STACK - 0x2)

	movl	$(REAL_STACK - 0x2), %eax /* switch to using the real mode stack */
	movl	%eax, %ebp
	movl	%eax, %esp

	/* Load the real mode GDT selectors */
	movw	$GDT_DATA_16, %ax
	movw	%ax, %ds
	movw	%ax, %es
	movw	%ax, %fs
	movw	%ax, %gs
	movw	$GDT_STACK_16, %ax
	movw	%ax, %ss

	ljmp	$GDT_CODE_16, $_not_quite_real

_not_quite_real:

	/* Switch of the protected mode flag */
	movl	%cr0, %eax				/* get current CR0 */
	andl	$0xfffffffe, %eax		/* clear the PE bit */
	movl	%eax, %cr0				/* and store it back. */

	/* Long jump to a real mode code segment */
	.byte	0x66
	ljmp	$0x1000, $_real_mode_segment

	.code16
_real_mode_segment:

	/* Clear the segment registers */
	movw	$0x1000, %ax
	movw	%ax, %ds
	movw	%ax, %es
	movw	%ax, %fs
	movw	%ax, %gs

	xorw	%ax, %ax
	movw	%ax, %ss

	/* Load the real mode IDT */
	lidt	(_real_idt)

	sti
	ret



/*
** Returns to protected mode from real mode
*/
	.globl switch_to_protected_mode
switch_to_protected_mode:
	.code16

	cli

	/* Load the protected mode IDT */
	lidt	(_idt_48)

	/* Set the protected mode flag */
	movl	%cr0, %eax				/* get current CR0 */
	orb		$0x1, %al				/* set the PE bit */
	movl	%eax, %cr0				/* and store it back. */
	
	/* Long jump to a real mode code segment */
	.byte	0x66
	.code32
	ljmp	$GDT_CODE, $(_protected_mode_segment + 0x10000)

_protected_mode_segment:

	/* Save the current return address */
	movl	(%esp), %eax
	movl	%eax, REAL_STACK

	/* Switch to the protected mode stack */
	movl	$RM_SAVED_ESP, %eax		/* restore saved ESP and EBP */
	movl	%eax, %esp
	movl	$RM_SAVED_EBP, %ebp

	movl	REAL_STACK, %eax		/* copy the return address to the stack */
	addl	$0x10000, %eax
	movl	%eax, (%esp)

	/* Load the real mode GDT selectors */
	movw	$GDT_DATA, %ax
	movw	%ax, %ds
	movw	%ax, %es
	movw	%ax, %fs
	movw	%ax, %gs
	movw	$GDT_STACK, %ax
	movw	%ax, %ss

	sti

	popal
	ret



	.code16
_real_idt:
	.short 0x3ff
	.long 0x0

_idt_48:
	.word	0x0800
	.quad	IDT_ADDRESS

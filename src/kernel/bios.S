/*
** File:	bios.S
**
** Author:	Sean Congden
**
** Description:	Utilities to run bios functions from protected mode.
*/

#define	__KERNEL__20113__
#define	__ASM__20113__

#include "bootstrap.h"
#include "vesa.h"

	.text
	.code32

/*
** Switches from protected mode to real mode.  Any context should be saved
** before calling this function.
**
** Currently disables interrupts while in real mode until we have an IDT
*/
	.globl switch_to_real_mode
switch_to_real_mode:	
	/* Disable interrupts */
	cli
	movb	$0x80, %al			/* disable NMIs */
	outb	%al, $0x70

	/* Save the current ESP and EBP values */
	movl	%esp, %eax
	movl	%eax, RM_SAVED_ESP
	movl	%ebp, RM_SAVED_EBP

	/* Set up the real mode stack */
	movl	(%ebp), %eax		/* copy the return address to the real stack */
	movl	%eax, REAL_STACK

	movl	$REAL_STACK, %eax	/* switch to using the real mode stack */
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
	movl	%cr0, %eax			/* get current CR0 */
	andl	$0xfffffffe, %eax	/* clear the PE bit */
	movl	%eax, %cr0			/* and store it back. */

	/* Long jump to a real mode code segment */
	ljmp	$GDT_CODE_16, $(_real_mode_segment)

	.code16
_real_mode_segment:

	/* Clear the segment registers */
	xorw	%ax, %ax
	movw	%ax, %ds
	movw	%ax, %es
	movw	%ax, %fs
	movw	%ax, %gs
	movw	%ax, %ss

	/* TODO: turn on interrupts! */
	/*   (we need a real mode IDT first) */
	ret


/*
** Returns to protected mode from real mode.
*/
	.globl return_to_protected_mode
return_to_protected_mode:
	/* TODO: Disable interrupts once real mode IDT is in place */
	
	/* Long jump to a real mode code segment */
	.byte	0x66
	.code32
	ljmp	$GDT_CODE, $(_protected_mode_segment)

_protected_mode_segment:

	/* Set the protected mode flag */
	movl	%cr0, %eax				/* get current CR0 */
	orl		$0x1, %eax				/* set the PE bit */
	movl	%eax, %cr0				/* and store it back. */

	/* Switch to the protected mode stack */
	movl	(%esp), %eax			/* save the return address */
	movl	%eax, REAL_STACK

	movl	$RM_SAVED_ESP, %eax		/* restore saved ESP and EBP */
	movl	%eax, %esp
	movl	$RM_SAVED_EBP, %ebp

	movl	$REAL_STACK, %eax		/* copy the return address to the stack */
	movl	%eax, (%esp)

	/* Load the real mode GDT selectors */
	movw	$GDT_DATA, %ax
	movw	%ax, %ds
	movw	%ax, %es
	movw	%ax, %fs
	movw	%ax, %gs
	movw	$GDT_STACK, %ax
	movw	%ax, %ss

	/* Re-enable interrupts */
	sti
	movb	$0x00, %al				/* re-enable NMIs */

	ret


/*
** Tests a vesa bios call by setting the video mode to a hardcoded value
*/
	.globl vesa_test
vesa_test:
	/* Save the current registers */
	pushal
	
	/* Switch to real mode */
	call switch_to_real_mode
	.code16

	/* Call the bios to set the desired mode (hardcoded) */
	movw $VESA_MODE_TO_LOAD, %bx
	movw $SET_MODE, %ax
	int $0x10
	cli 						/* make sure interrupts remain disabled */

	/* Switch back to protected mode */
	call return_to_protected_mode
	.code32

	/* Restore the saved registers */
	popal
	ret

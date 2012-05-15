/*
** Support routines needed for boot up and the console.
*/
ARG1	= 8			// Offset to 1st argument
ARG2	= 12			// Offset to 2nd argument

/*
** Name:	__inb, __inw, __inl
**
** Description: read a single byte, word, or longword from the specified
**		input port
** Arguments:	the port number
** Returns:	the byte, word, or longword value from the port (unused
**		high-order bytes are zeroed)
*/
	.globl	__inb, __inw, __inl

__inb:
	enter	$0,$0
	xorl	%eax,%eax	// Clear the high order bytes of %eax
	movl	ARG1(%ebp),%edx	// Move port number to %edx
	inb	(%dx)		// Get a byte from the port into %al (low
	leave			//   byte of %eax)
	ret
__inw:
	enter	$0,$0
	xorl	%eax,%eax	// Clear the high order bytes of %eax
	movl	ARG1(%ebp),%edx	// Move port number to %edx
	inw	(%dx)		// Get a word from the port into %ax (low
	leave			//   word of %eax)
	ret
__inl:
	enter	$0,$0
	xorl	%eax,%eax	// Clear the high order bytes of %eax
	movl	ARG1(%ebp),%edx	// Move port number to %edx
	inl	(%dx)		// Get a longword from the port into %eax
	leave
	ret
	
/*
** Name:	__outb, __outw, __outl
**
** Description: write a single byte, word, or longword to the specified
**		output port
** Arguments:	the port number, the value to write to it (unused high-order
**		bytes are ignored)
*/
	.globl	__outb, __outw, __outl
__outb:
	enter	$0,$0
	movl	ARG1(%ebp),%edx	// Get the port number into %edx,
	movl	ARG2(%ebp),%eax	//   and the value into %eax
	outb	(%dx)		// Output that byte to the port
	leave			//   (only %al is sent)
	ret
__outw:
	enter	$0,$0
	movl	ARG1(%ebp),%edx	// Get the port number into %edx,
	movl	ARG2(%ebp),%eax	//   and the value into %eax
	outw	(%dx)		// Output that word to the port.
	leave			//   (only %ax is sent)
	ret
__outl:
	enter	$0,$0
	movl	ARG1(%ebp),%edx	// Get the port number into %edx,
	movl	ARG2(%ebp),%eax	//   and the value into %eax
	outl	(%dx)		// Output that longword to the port.
	leave
	ret

/*
** __get_flags: return the current processor flags
**	int __get_flags( void );
*/
	.globl	__get_flags

__get_flags:
	pushfl			// Push flags on the stack,
	popl	%eax		//   and pop them into eax.
	ret

/*
** __pause: halt until something happens
**      void __pause( void );
*/
	.globl	__pause

__pause:
	enter	$0,$0
	sti
	hlt
	leave
	ret

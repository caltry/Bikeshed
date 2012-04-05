/*
** sccs id:	@(#)klibs.S	1.1	4/5/12
**
** File:	klibs.S
**
** Author:	4003-506 class of 20113
**
** Contributor:
**
** Description:	Assembly implementations of kernel library functions
*/

#define	__KERNEL__20113__
#define	__ASM__20113__

/*
** _get_ebp - return current contents of EBP at the time of the call
**
** Could be used, e.g., by _kpanic
*/

	.globl	_get_ebp
_get_ebp:
	movl	%ebp, %eax
	ret

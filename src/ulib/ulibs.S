/*
** sccs id:	@(#)ulibs.S	1.1	4/5/12
**
** File:	ulibs.S
**
** Author:	4003-506 class of 20113
**
** Contributor:
**
** Description:	Assembly implementations of user-level library functions
*/

#define	__ASM__20113__

#include "../kernel/syscalls.h"

/*
** System call stubs
**
** All have the same structure:
**
**      move a code into EAX
**      generate the interrupt
**      return to the caller
**
** As these are simple "leaf" routines, we don't use
** the standard enter/leave method to set up a stack
** frame - that takes time, and we don't really need it.
*/

#define SYSCALL(name) \
	.globl	name              ; \
name:                             ; \
	movl	$SYS_##name, %eax ; \
	int	$INT_VEC_SYSCALL  ; \
	ret

/*
** for debugging purposes, this could be changed to expand into
** (e.g., for write) something like this:
**
**      .globl  write
**      .globl  c_printf
** write:       movl    $SYS_write, %eax
**      int     $INT_VEC_SYSCALL
**      pushl   $write_fmt
**      call    c_printf
**      addl    $4, %esp
**      ret
** write_fmt: .string "write() is returning to %08x after printing %08x\n"
*/

SYSCALL(fork)
SYSCALL(exit)
SYSCALL(read)
SYSCALL(write)
SYSCALL(msleep)
SYSCALL(kill)
SYSCALL(get_priority)
SYSCALL(get_pid)
SYSCALL(get_ppid)
SYSCALL(get_time)
SYSCALL(get_state)
SYSCALL(set_priority)
SYSCALL(set_time)
SYSCALL(exec)
SYSCALL(sem_init)
SYSCALL(sem_destroy)
SYSCALL(sem_post)
SYSCALL(sem_wait)
SYSCALL(sem_try_wait)
SYSCALL(lock_init)
SYSCALL(lock_destroy)
SYSCALL(lock_lock)
SYSCALL(lock_unlock)
SYSCALL(message_send)
SYSCALL(message_receive)
SYSCALL(message_try_receive)
SYSCALL(message_has_message)
SYSCALL(fs_read)
SYSCALL(fs_write)

// This is a bogus system call; it's here so that we can test
// our handling of out-of-range syscall codes in the syscall ISR.

SYSCALL(bogus)

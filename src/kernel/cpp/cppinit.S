/* Initialize C++ support by calling the C++ static object constructors */

.globl __cpp_init
__cpp_init:
/* Setup the C++ Support */
	mov $start_ctors, %ecx
	jmp skip_constructor
call_constructor:
	call *(%ecx)
	add $4, %ecx
skip_constructor:
	cmp $end_ctors, %ecx
	jb call_constructor
/* End C++ setup support */
	ret

.globl __cpp_deinit
__cpp_deinit:
/* Cleanup the C++ support */
	mov $end_dtors, %ebx
	jmp skip_destructors	
call_destructor:
	sub $4, %ebx
	call *(%ebx)
skip_destructors:
	cmp $start_dtors, %ebx
	ja call_destructor
/* End cleanup of C++ support */
	ret


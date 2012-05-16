.global _start
.extern main
.extern exit
_start:

/* For now we don't have an argc, or argv */
pushl $0x0
pushl $0x0
call main
add $8, %esp

/* Kill the process */
call exit

/* If we return before the process has been reclaimed */
wait_to_be_killed:
	hlt
	jmp wait_to_be_killed

.data
message:
.string "Welcome to Bikeshed OS"

.text
put_char:
	pushl %eax
	pushl %ebx

	movl 12(%esp), %eax
	movw 16(%esp), %bx
	movw %bx, (%eax)

	popl %ebx
	popl %eax
	ret

/* Writes a string to video memory */
put_string:
	pushl %eax
	pushl %ebx
	pushl %ecx

	movl 16(%esp), %eax	
	movl $0xB8000, %ecx
	movb $0x70, %bh	

loop:
	movb (%eax), %bl
	testb %bl, %bl
	jz return

	pushl %ebx
	pushl %ecx
	call put_char
	add $8, %esp

	inc %eax
	add $2, %ecx
	jmp loop

return:
	popl %ecx
	popl %ebx
	popl %eax
	ret

.globl main 
main:
	pushl $message
	call put_string
	add $4, %esp

wait:
	hlt
	jmp wait

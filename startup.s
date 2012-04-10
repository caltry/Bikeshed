# 1 "startup.S"
# 1 "<built-in>"
# 1 "<command-line>"
# 1 "startup.S"
# 20 "startup.S"
# 1 "bootstrap.h" 1
# 21 "startup.S" 2
# 39 "startup.S"
.section .data
.align 4096
.globl boot_page_directory
boot_page_directory:
 .long 0x00000083

 .fill ((0x00010000 >> 22) - 1), 4, 0

 .long 0x00000083

 .fill (1024 - (0x00010000 >> 22) - 1), 4, 0





 .globl begtext

 .text
begtext:




 .globl _start

_start:
 cli
 movb $0x00, %al
 outb $0x70


 mov $boot_page_directory, %ecx
 mov %ecx, %cr3

 mov %cr4, %ecx
 or $0x00000010, %ecx
 mov %ecx, %cr4

 mov %cr0, %ecx
 or $0x80000000, %ecx
 mov %ecx, %cr0


 lea enable_paging, %ecx
 jmp *%ecx
enable_paging:






 xorl %eax, %eax
 movw $0x0018, %ax
 movw %ax, %ds
 movw %ax, %es
 movw %ax, %fs
 movw %ax, %gs

 movw $0x0020, %ax
 movw %ax, %ss

 movl $0x00010000, %ebp
 movl %ebp, %esp
# 112 "startup.S"
 .globl __bss_start,_end

 movl $__bss_start,%edi
clearbss:
 movl $0,(%edi)
 addl $4,%edi
 cmpl $_end,%edi
 jb clearbss
# 128 "startup.S"
 call _init
# 137 "startup.S"
 jmp __isr_restore
# 174 "startup.S"
ARG1 = 8
ARG2 = 12
# 186 "startup.S"
 .globl __inb, __inw, __inl

__inb:
 enter $0,$0
 xorl %eax,%eax
 movl ARG1(%ebp),%edx
 inb (%dx)
 leave
 ret
__inw:
 enter $0,$0
 xorl %eax,%eax
 movl ARG1(%ebp),%edx
 inw (%dx)
 leave
 ret
__inl:
 enter $0,$0
 xorl %eax,%eax
 movl ARG1(%ebp),%edx
 inl (%dx)
 leave
 ret
# 218 "startup.S"
 .globl __outb, __outw, __outl
__outb:
 enter $0,$0
 movl ARG1(%ebp),%edx
 movl ARG2(%ebp),%eax
 outb (%dx)
 leave
 ret
__outw:
 enter $0,$0
 movl ARG1(%ebp),%edx
 movl ARG2(%ebp),%eax
 outw (%dx)
 leave
 ret
__outl:
 enter $0,$0
 movl ARG1(%ebp),%edx
 movl ARG2(%ebp),%eax
 outl (%dx)
 leave
 ret





 .globl __get_flags

__get_flags:
 pushfl
 popl %eax
 ret





 .globl __pause

__pause:
 enter $0,$0
 sti
 hlt
 leave
 ret

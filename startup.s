# 1 "startup.S"
# 1 "<built-in>"
# 1 "<command-line>"
# 1 "startup.S"
# 20 "startup.S"
# 1 "bootstrap.h" 1
# 21 "startup.S" 2
# 32 "startup.S"
 .globl begtext

 .text
begtext:




 .globl _start

_start:
 cli
 movb $0x00, %al
 outb $0x70





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
# 71 "startup.S"
 .globl __bss_start,_end

 movl $__bss_start,%edi
clearbss:
 movl $0,(%edi)
 addl $4,%edi
 cmpl $_end,%edi
 jb clearbss
# 87 "startup.S"
 call _init
# 96 "startup.S"
 jmp __isr_restore
# 132 "startup.S"
ARG1 = 8
ARG2 = 12
# 144 "startup.S"
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
# 176 "startup.S"
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

#
# User compilation/assembly definable options
#
#	ISR_DEBUGGING_CODE	include context restore debugging code
#	CLEAR_BSS_SEGMENT	include code to clear all BSS space
#	SP2_CONFIG		enable SP2-specific startup variations
#	REPORT_MYSTERY_INTS	print a message on interrupt 0x27
#
USER_OPTIONS = -DCLEAR_BSS_SEGMENT -DSP2_CONFIG -DISR_DEBUGGING_CODE

#
# YOU SHOULD NOT NEED TO CHANGE ANYTHING BELOW THIS POINT!!!
#
# Compilation/assembly control
#

INCLUDES = -I.

DYN_SRC_FILES = $(shell ls *.[chsS] *.cpp 2> /dev/null)
DYN_OBJ_FILES = $(shell ls *.[csS] *.cpp 2> /dev/null | sed -e 's/^\(.*\).[csS]$$/\1.o/g' -e 's/^\(.*\).cpp$$/\1.o/g')

#
# Compilation/assembly/linking commands and options
#
CPP = cpp
# CPPFLAGS = $(USER_OPTIONS) -nostdinc -I- $(INCLUDES)
CPPFLAGS += $(USER_OPTIONS) -nostdinc $(INCLUDES)

CC = gcc
CFLAGS += -std=gnu99 -Wshadow -fno-stack-protector -fno-builtin -Wall -Wextra 
CFLAGS += -Wstrict-prototypes -m32 $(CPPFLAGS)

CXX = g++
CXXFLAGS += -m32 -Wshadow -nostdlib -fno-builtin -nostartfiles -nodefaultlibs -fno-exceptions -fno-rtti -fno-stack-protector -Wall -Wextra

AS = as
ASFLAGS = --32 -n32 

LD = ld
LDFLAGS = -m elf_i386

LN := ln -f

AR := ar -cr

#		
# Transformation rules - these ensure that all compilation
# flags that are necessary are specified
#
# Note use of 'cpp' to convert .S files to temporary .s files: this allows
# use of #include/#define/#ifdef statements. However, the line numbers of
# error messages reflect the .s file rather than the original .S file. 
# (If the .s file already exists before a .S file is assembled, then
# the temporary .s file is not deleted.  This is useful for figuring
# out the line numbers of error messages, but take care not to accidentally
# start fixing things by editing the .s file.)
#

.SUFFIXES:	.S .b

.c.s:
	$(CC) $(CFLAGS) -S $*.c

.S.s:
	$(CPP) $(CPPFLAGS) -o $*.s $*.S

.S.o:
	$(CPP) $(CPPFLAGS) -o $*.s $*.S
	$(AS) $(ASFLAGS) -o $*.o $*.s -a=$*.lst
	$(RM) -f $*.s

.s.b:
	$(AS) $(ASFLAGS) -o $*.o $*.s -a=$*.lst
	$(LD) $(LDFLAGS) -Ttext 0x0 -s --oformat binary -e begtext -o $*.b $*.o

.c.o:
	$(CC) $(CFLAGS) -c $*.c

.cpp.o:
	$(CXX) $(CXXFLAGS) -c $*.cpp

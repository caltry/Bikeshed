#
# SCCS ID: %W%	%G%
#
# Makefile to control the compiling, assembling and linking of
# standalone programs in the DSL.  Used for both 4003-406 and
# 4003-506 (with appropriate tweaking).
#

# Modules must end with a trailing slash.
MODULES := applications/ boot/ kernel/lib/ kernel/ serial/ ulib/ kernel/memory/ kernel/vm8086/ kernel/bios/ kernel/pci/ kernel/data_structures/ kernel/network/ kernel/cpp/ fs/ fs/ext2/ kernel/video/

# That's right, there's no default support for newlines in Make.
define NL


endef

# Tell the linker to generate i386 bytecode
LD = ld
LDFLAGS = -m elf_i386

#
# User supplied files
#
U_C_SRC = clock.c klibc.c pcbs.c queues.c scheduler.c sio.c stacks.c syscalls.c system.c ulibc.c users.c 
U_C_OBJ = clock.o klibc.o pcbs.o queues.o scheduler.o sio.o stacks.o syscalls.o system.o ulibc.o users.o
U_S_SRC = klibs.S ulibs.S
U_S_OBJ = klibs.o ulibs.o
U_LIBS	= $(shell find $(MODULES) \( ! -regex '.*/\..*' \) -type f -name "*.ar")

default:
	$(MAKE) make_modules
	$(MAKE) prog.b
	$(MAKE) real.b

# Build all of the submodules that we've defined.
.PHONY: make_modules
make_modules:
	$(MAKE) $(MODULES)

# Build the default rule on a subdirectory. FORCE is an old hack from before
# .PHONY existed. I need to use it here because you can't declare wildcard
# recipes as PHONY. This basically forces us to always recursively build
# subdierectories.
%/: FORCE
	$(MAKE) -C $@
FORCE:

# Binary/source file for system bootstrap code

BOOT_OBJ = bootstrap.b
BOOT_SRC = bootstrap.S

# Assembly language object/source files

S_OBJ = startup.o isr_stubs.o $(U_S_OBJ)
S_SRC =	startup.S isr_stubs.S $(U_S_SRC)

# C object/source files

C_OBJ =	c_io.o support.o $(U_C_OBJ)
C_SRC =	c_io.c support.c $(U_C_SRC)

# Collections of files

#OBJECTS = $(S_OBJ) $(C_OBJ)

SOURCES = $(BOOT_SRC) $(S_SRC) $(C_SRC)


prog.out: $(OBJECTS)
	$(LD) $(LDFLAGS) -o prog.out $(OBJECTS)

prog.o:	$(OBJECTS) make_modules
	$(LD) $(LDFLAGS) -o prog.o -Tkernel.ld $(OBJECTS) $(U_LIBS) $(U_LIBS)

prog.b:	prog.o
	$(LD) $(LDFLAGS) -o prog.b -s --oformat binary -Tlinker.ld prog.o

real.b:	prog.o
	$(LD) $(LDFLAGS) -o real.b -s --oformat binary -Treal.ld prog.o

Offsets:	Offsets.c
	$(CC) $(INCLUDES) -o Offsets Offsets.c

#
# Clean out this directory
#

clean:
	rm -f *.nl *.lst *.b *.o *.image *.dis BuildImage Offsets
	$(foreach module,$(MODULES),$(MAKE) -C $(module) clean$(NL))

#
# Create a printable namelist from the prog.o file
#
# options to 'nm':
#	B	use (traditional) BSD format
#	n	sort by addresses, not by names
#	g	only global symbols
#

prog.nl: prog.o
#	nm -Bng prog.o | pr -w80 -3 > prog.nl
	nm -Bn prog.o | pr -w80 -3 > prog.nl

#
# Generate a disassembly
#
# options to 'objdump':
#	d	disassemble all text sections
#

prog.dis: prog.o
#	dis prog.o > prog.dis
	objdump -d prog.o > prog.dis

#
#       makedepend is a program which creates dependency lists by
#       looking at the #include lines in the source files
#

.PHONY: depend
depend:
	$(foreach module, $(MODULES),$(MAKE) -C $(module) $@ $(NL))

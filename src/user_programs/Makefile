
USER_PROGRAMS := init/ welcome/ desktop_process/

export INCLUDES = -I. -I$(shell pwd)/../ulib/ -I$(shell pwd)/../include/ -I$(shell pwd)/../
export CFLAGS = -s --std=gnu99 -m32 -Wall -Werror -fno-builtin -nodefaultlibs -nostdinc -nostdlib -T$(shell pwd)/clinker.ld
export CC = gcc

export CXX = g++
export CXXFLAGS = -s -m32 -Wall -Werror -fno-builtin -nodefaultlibs -nostdinc -nostdlib -T$(shell pwd)/clinker.ld

export LD = ld
export LDFLAGS = -Tclinker.ld

export AS = as
export ASFLAGS = --32 -n32

export USER_LIB_OBJS = $(shell pwd)/crt0.o $(shell pwd)/../ulib/*.o

# That's right, there's no default support for newlines in Make.
define NL


endef

all: crt0 make_user_programs

.PHONY: make_user_programs
make_user_programs:
	$(MAKE) $(USER_PROGRAMS)

# Build the default rule on a subdirectory. FORCE is an old hack from before
# .PHONY existed. I need to use it here because you can't declare wildcard
# recipes as PHONY. This basically forces us to always recursively build
# subdierectories.
%/: FORCE
	$(MAKE) -C $@
FORCE:

crt0: 
	$(AS) $(ASFLAGS) -c crt0.S -o crt0.o

clean:
	rm -f *.o	
	$(foreach module,$(USER_PROGRAMS),$(MAKE) -C $(module) clean$(NL))

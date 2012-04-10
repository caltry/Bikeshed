include ../defs.mk

CFLAGS += -I../boot -I../include -I../kernel
CPPFLAGS += -I../kernel -I../include

SRC_FILES = $(DYN_SRC_FILES)
OBJ_FILES = $(DYN_OBJ_FILES)

.PHONY: all

all: ulib.ar

ulib.ar: $(OBJ_FILES)
	$(AR) $@ $^

clean:
	$(RM) *.o *.lst *.ar

make depend:
	makedepend -- $(CFLAGS) -- $(SRC_FILES)
	rm Makefile.bak
# DO NOT DELETE THIS LINE -- make depend depends on it.

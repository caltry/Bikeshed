include ../defs.mk

CFLAGS += -I../boot -I../include -I../kernel
CPPFLAGS += -I../kernel -I../include

OBJ_FILES := c_io.o support.o ulibc.o ulibs.o

.PHONY: all

all: ulib.ar

ulib.ar: $(OBJ_FILES)
	$(AR) $@ $^

clean:
	$(RM) *.o *.lst *.ar

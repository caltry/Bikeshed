include ../defs.mk

CFLAGS += -I../include -I../ulib/ -I../kernel -I../kernel/lib -I../boot

SRC_FILES = $(DYN_SRC_FILES)
OBJ_FILES = $(DYN_OBJ_FILES)

.PHONY: all

all: serial.ar

serial.ar: $(OBJ_FILES)
	$(AR) $@ $^

clean:
	$(RM) *.o *.lst *.ar

make depend:
	makedepend -- $(CFLAGS) -- $(SRC_FILES)
	rm Makefile.bak
# DO NOT DELETE THIS LINE -- make depend depends on it.

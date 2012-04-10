include ../defs.mk

SRC_FILES = $(DYN_SRC_FILES)
# Omit bootstrap.b, it's special
OBJ_FILES := startup.o

all: bootstrap.b boot.ar

boot.ar: $(OBJ_FILES)
	$(AR) $@ $^

clean:
	$(RM) *.lst *.b *.o *.ar

make depend:
	makedepend -- $(CFLAGS) -- $(SRC_FILES)
	rm Makefile.bak
# DO NOT DELETE THIS LINE -- make depend depends on it.

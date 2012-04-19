include ../defs.mk

CFLAGS += -I../include -I../ulib -I../kernel -I../kernel/lib -I../boot
CFLAGS += -I../serial -I../applications -I..
CPPFLAGS += -I../boot

SRC_FILES = $(DYN_SRC_FILES)
OBJ_FILES = $(DYN_OBJ_FILES)

.PHONY: all klib.ar

all: klib.ar kernel.ar

kernel.ar: $(OBJ_FILES)
	$(AR) $@ $^

clean:
	$(RM) *.o *.lst *.ar *.s

.PHONY: depend
depend: makedepends.mk
makedepends.mk: $(SRC_FILES)
	makedepend -f- -- $(CFLAGS) -- $(SRC_FILES) > makedepends.mk

include makedepends.mk

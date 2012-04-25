include ../defs.mk

CFLAGS += -I../include -I../ulib -I../kernel -I../kernel/lib -I../kernel/video -I../boot
CFLAGS += -I../serial -I../applications
CPPFLAGS += -I../boot -I../kernel/video

SRC_FILES = $(DYN_SRC_FILES)
OBJ_FILES = $(DYN_OBJ_FILES)

.PHONY: all klib.ar video.ar

all: klib.ar video.ar kernel.ar

kernel.ar: $(OBJ_FILES)
	$(AR) $@ $^

clean:
	$(RM) *.o *.lst *.ar *.s

.PHONY: depend
depend: makedepends.mk
makedepends.mk: $(SRC_FILES)
	makedepend -f- -- $(CFLAGS) -- $(SRC_FILES) > makedepends.mk

include makedepends.mk

include ../defs.mk

CFLAGS += -I../include -I../kernel -I../kernel/lib -I../kernel/video -I../boot
CFLAGS += -I../serial -I../applications -I../kernel/bios -I.. -I../fs
CPPFLAGS += -I../boot -I../kernel/video

CXXFLAGS += -I../include -I../kernel -I../kernel/lib -I../boot
CXXFLAGS += -I../serial -I../applications -I ../kernel/bios

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

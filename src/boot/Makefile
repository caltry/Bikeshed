include ../defs.mk

SRC_FILES = $(DYN_SRC_FILES)
# Omit bootstrap.b, it's special
OBJ_FILES := startup.o support.o

CPPFLAGS += -I../kernel/video

all: bootstrap.b boot.ar

boot.ar: $(OBJ_FILES)
	$(AR) $@ $^

clean:
	$(RM) *.lst *.b *.o *.ar *.s

.PHONY: depend
depend: makedepends.mk
makedepends.mk: $(SRC_FILES)
	makedepend -f- -- $(CFLAGS) -- $(SRC_FILES) > makedepends.mk

include makedepends.mk

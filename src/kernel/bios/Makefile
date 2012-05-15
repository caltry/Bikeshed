include ../../defs.mk

CFLAGS += -I../../include -I../../boot -I.. -I../../kernel/lib
CPPFLAGS += -I../../include -I../../boot -I.. -I../../kernel/lib

SRC_FILES = $(DYN_SRC_FILES) realmode.S
OBJ_FILES = $(DYN_OBJ_FILES)

.PHONY: all

all: bios.ar realmode.o

bios.ar: realmode.o
	$(AR) $@ $^

clean:
	$(RM) *.o *.lst *.ar

.PHONY: depend
depend: makedepends.mk
makedepends.mk: $(SRC_FILES)
	makedepend -f- -- $(CFLAGS) -- $(SRC_FILES) > makedepends.mk

include makedepends.mk

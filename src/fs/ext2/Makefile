include ../../defs.mk

CFLAGS += -I../../include

SRC_FILES = $(DYN_SRC_FILES)
OBJ_FILES = $(DYN_OBJ_FILES)
EXCLUDE_FROM_ARCHIVE := compile_time_tests.o

.PHONY: all

all: ext2.ar

ext2.ar: $(OBJ_FILES)
	$(AR) $@ $^
	ar -d $@ $(EXCLUDE_FROM_ARCHIVE)

clean:
	$(RM) *.o *.lst *.ar

.PHONY: depend
depend: .makedepends.mk
.makedepends.mk: $(SRC_FILES)
	makedepend -f- -- $(CFLAGS) -- $(SRC_FILES) > .makedepends.mk

include .makedepends.mk

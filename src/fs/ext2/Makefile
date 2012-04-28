include ../../defs.mk

INCLUDES += -I../../include -I../..

ifdef RAMDISK_VIRT_LOCATION
CFLAGS += -DRAMDISK_VIRT_LOCATION=${RAMDISK_VIRT_LOCATION}
endif

ifdef RAMDISK_PHYS_LOCATION
CFLAGS += -DRAMDISK_PHYS_LOCATION=${RAMDISK_PHYS_LOCATION}
endif

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

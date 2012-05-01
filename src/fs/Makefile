include ../defs.mk
include ../fs_defs.mk

INCLUDES += -I../include -I../kernel -I../

ifdef RAMDISK_VIRT_LOCATION
CFLAGS += -DRAMDISK_VIRT_LOCATION=${RAMDISK_VIRT_LOCATION}
endif

ifdef RAMDISK_PHYS_LOCATION
CFLAGS += -DRAMDISK_PHYS_LOCATION=${RAMDISK_PHYS_LOCATION}
endif

ifdef RAMDISK_SIZE_KiB
CFLAGS += -DRAMDISK_SIZE_KiB=${RAMDISK_SIZE_KiB}
endif

SRC_FILES = $(DYN_SRC_FILES)
OBJ_FILES = $(DYN_OBJ_FILES)

.PHONY: all

all: fs.ar

fs.ar: $(OBJ_FILES)
	$(AR) $@ $^

clean:
	$(RM) *.o *.lst *.ar

.PHONY: depend
depend: makedepends.mk
makedepends.mk: $(SRC_FILES)
	makedepend -f- -- $(CFLAGS) -- $(SRC_FILES) > makedepends.mk

include makedepends.mk

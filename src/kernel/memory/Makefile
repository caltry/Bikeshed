include ../../defs.mk

CFLAGS += -I../../include -I../../ulib -I..

OBJ_FILES := kmalloc.o physical.o paging.o memory.o

.PHONY: all

all: kmem.ar

kmem.ar: $(OBJ_FILES)
	$(AR) $@ $^

clean:
	$(RM) *.o *.lst *.ar

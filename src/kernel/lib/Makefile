include ../../defs.mk

CFLAGS += -I../../include -I../../ulib -I..

OBJ_FILES := klibc.o klibs.o

.PHONY: all

all: klib.ar

klib.ar: $(OBJ_FILES)
	$(AR) $@ $^

clean:
	$(RM) *.o *.lst *.ar

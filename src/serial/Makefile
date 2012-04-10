include ../defs.mk

CFLAGS += -I../include -I../ulib/ -I../kernel -I../kernel/lib -I../boot

OBJ_FILES := sio.o

.PHONY: all

all: serial.ar

serial.ar: $(OBJ_FILES)
	$(AR) $@ $^

clean:
	$(RM) *.o *.lst *.ar

include ../defs.mk

CFLAGS += -I../include -I../ulib/ -I../kernel

OBJ_FILES := users.o

.PHONY: all

all: users.ar

users.ar: $(OBJ_FILES)
	$(AR) $@ $^

clean:
	$(RM) *.o *.lst *.ar

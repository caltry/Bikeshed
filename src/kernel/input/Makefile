include ../../defs.mk

CFLAGS += -I../../include -I../../kernel -I../../kernel/lib -I../../boot -I..

SRC_FILES = $(DYN_SRC_FILES)
OBJ_FILES = $(DYN_OBJ_FILES)

.PHONY: all

all: input.ar

input.ar: $(OBJ_FILES)
	$(AR) $@ $^

clean:
	$(RM) *.o *.lst *.ar

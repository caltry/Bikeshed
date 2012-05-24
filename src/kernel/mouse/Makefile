include ../../defs.mk

CFLAGS += -I../../include -I../../kernel/lib -I.. -I../../kernel/video -I../../boot

SRC_FILES = $(DYN_SRC_FILES)
OBJ_FILES = $(DYN_OBJ_FILES)

.PHONY: all

all: mouse.ar

mouse.ar: $(OBJ_FILES)
	$(AR) $@ $^

clean:
	$(RM) *.o *.lst *.ar

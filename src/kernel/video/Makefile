include ../../defs.mk

CFLAGS += -I../../include -I../../ulib -I../../kernel/lib -I..

OBJ_FILES := vesa.o

.PHONY: all

all: video.ar

video.ar: $(OBJ_FILES)
	$(AR) $@ $^

clean:
	$(RM) *.o *.lst *.ar

include ../../defs.mk

CFLAGS += -I../../include -I../../ulib -I../../kernel/lib -I.. -I../../kernel/bios

OBJ_FILES := vesa.o

.PHONY: all

all: video.ar

video.ar: $(OBJ_FILES)
	$(AR) $@ $^

clean:
	$(RM) *.o *.lst *.ar

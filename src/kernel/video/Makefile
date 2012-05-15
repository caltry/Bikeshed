include ../../defs.mk

CFLAGS += -I../../include -I../../ulib -I../../kernel/lib -I.. -I../../kernel/bios -I../../applications

OBJ_FILES := vesa.o video.o graphics.o gconsole.o

.PHONY: all

all: video.ar

video.ar: $(OBJ_FILES)
	$(AR) $@ $^

clean:
	$(RM) *.o *.lst *.ar

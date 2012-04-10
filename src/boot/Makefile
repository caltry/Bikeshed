include ../defs.mk

# Omit bootstrap.b, it's special
OBJ_FILES := startup.o

CPPFLAGS += -I../kernel/video

all: bootstrap.b boot.ar

boot.ar: $(OBJ_FILES)
	$(AR) $@ $^

clean:
	$(RM) *.lst *.b *.o *.ar

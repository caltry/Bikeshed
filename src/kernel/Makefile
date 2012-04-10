include ../defs.mk

CFLAGS += -I../include -I../ulib -I../kernel -I../kernel/lib -I../kernel/video -I../boot
CFLAGS += -I../serial -I../applications
CPPFLAGS += -I../boot

OBJ_FILES := clock.o isr_stubs.o pcbs.o queues.o scheduler.o stacks.o syscalls.o system.o

.PHONY: all klib.ar video.ar

all: klib.ar video.ar kernel.ar

kernel.ar: $(OBJ_FILES)
	$(AR) $@ $^

clean:
	$(RM) *.o *.lst *.ar

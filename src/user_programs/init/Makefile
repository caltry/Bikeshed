
all:
	gcc -std=gnu99 -m32 -Wall -Werror -o initproc -nostdinc -nostdlib ../crt0.o ../../ulib/ulib*.o main.c
	cp initproc ../../../build/ramdisk_staging/etc/.

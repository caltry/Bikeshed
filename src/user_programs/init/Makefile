
all: init asm 

init: crt0
	gcc --std=gnu99 -m32 -Wall -Werror -fno-builtin -nodefaultlibs -nostdinc -nostdlib -c main.c
	gcc -m32 -fno-exceptions -nodefaultlibs -fno-builtin -nostdinc -nostdlib -Tclinker.ld ../crt0.o main.o ../../ulib/ulib*.o -o initproc
	strip initproc
	cp initproc ../../../build/ramdisk_staging/etc/.

asm: crt0
	gcc --std=gnu99 -m32 -Wall -Werror -nostdinc -nostdlib -c init.S
	gcc -m32 -fno-exceptions -nodefaultlibs -fno-builtin -nostdinc -nostdlib ../crt0.o ../../ulib/ulib*.o init.o -o welcome
	strip welcome
	cp welcome ../../../build/ramdisk_staging/etc/.

crt0:
	gcc -m32 -Wall -Werror -fno-exceptions -nodefaultlibs -nostdinc -nostdlib -c ../crt0.S

# ld -Tclinker.ld ../crt0.o main.o ../../ulib/ulib*.o -o initproc


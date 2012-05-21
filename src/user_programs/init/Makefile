

all:
	$(CC) $(CFLAGS) $(INCLUDES) $(USER_LIB_OBJS) main.c -o initproc
	cp initproc ../../../build/ramdisk_staging/etc/.

clean:
	rm -f *.o
	rm -f initproc

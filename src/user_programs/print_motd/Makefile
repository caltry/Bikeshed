all:
	$(CC) $(CFLAGS) $(INCLUDES) $(USER_LIB_OBJS) main.c -o print_motd
	cp print_motd ../../../build/ramdisk_staging/etc/.

clean:
	rm -f *.o
	rm -f initproc

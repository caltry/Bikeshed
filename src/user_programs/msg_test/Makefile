

all:
	$(CC) $(CFLAGS) $(INCLUDES) $(USER_LIB_OBJS) msg_test.c -o msg_test
	cp msg_test ../../../build/ramdisk_staging/etc/.

clean:
	rm -f *.o
	rm -f msg_test

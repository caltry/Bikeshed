

all:
	$(AS) $(ASFLAGS) $(INCLUDES) welcome.S -o welcome.o
	$(CC) $(CFLAGS) $(INCLUDES) $(USER_LIB_OBJS) welcome.o -o welcome
	cp welcome ../../../build/ramdisk_staging/etc/.

clean:
	rm -f *.o
	rm -f welcome

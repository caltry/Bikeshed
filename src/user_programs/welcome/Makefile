

all:
	$(AS) $(ASFLAGS) $(INCLUDES) -c welcome.S -o welcome.o
	$(CC) $(CFLAGS) $(INCLUDES) $(USER_LIB_OBJS) welcome.o -o welcome

clean:
	rm -f *.o
	rm -f welcome

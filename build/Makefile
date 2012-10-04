CFLAGS=-Wall -Werror -Wextra -pedantic -std=gnu99

all: FancyCat

FancyCat: FancyCat.o
	$(CC) $(CFLAGS) -o FancyCat $^

.PHONY: populate_ramdisk
populate_ramdisk: bikeshed_fs
	debugfs -w -f debugfs_commands.debugfs $^

clean:
	$(RM) BuildImage BuildImage.o
	$(RM) FancyCat FancyCat.o

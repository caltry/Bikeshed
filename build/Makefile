CFLAGS=-Wall -Werror -Wextra -pedantic -std=gnu99

BuildImage: BuildImage.o
	$(CC) $(CFLAGS) -o BuildImage $^

FancyCat: FancyCat.o
	$(CC) $(CFLAGS) -o FancyCat $^

clean:
	$(RM) BuildImage BuildImage.o
	$(RM) FancyCat FancyCat.o

/* This file contains the init process!
*/

typedef unsigned short uint16_t;

#define VIDEO_BASE_ADDR 0xB8000
#define	SCREEN_MIN_X	0
#define	SCREEN_MIN_Y	0
#define	SCREEN_X_SIZE	80
#define	SCREEN_Y_SIZE	25
#define	SCREEN_MAX_X	( SCREEN_X_SIZE - 1 )
#define	SCREEN_MAX_Y	( SCREEN_Y_SIZE - 1 )
#define	VIDEO_ADDR(x,y)	( unsigned short * ) \
		( VIDEO_BASE_ADDR + 2 * ( (y) * SCREEN_X_SIZE + (x) ) )
void put_char(int x, int y, char c)
{
	uint16_t* addr = VIDEO_ADDR(x, y);
	*addr = (0x70 << 8) | c;
}

void put_string(int x, int y, const char* message)
{
	while (*message != 0)
	{
		put_char(x, y, *message);

		++x; if (x > 80) { x = 0; ++y; if (y == 25) { y = 0; } }

		++message;
	}
}

//extern void set_time(unsigned int time);
extern unsigned int fork(unsigned int* pid);
extern unsigned int exec(const char* path);
int main()
{
	put_string(0, 0, "Loaded an ELF and running C code!");
	put_string(0, 1, "Program");


	unsigned int pid = -1;
	if (fork(&pid) == 0)
	{
		if (pid == 0)
		{
			// Child
			put_string(10, 0, "CHILD!");
			if (fork(&pid) == 0)
			{
				if (pid == 0)
				{
					exec("/welcome");
					put_string(40, 0, "Child returned!?");
				} else {
					put_string(60, 0, "Fork 2!");
				}
			}
		} else {
			// Parent
			put_string(20, 0, "PARENT!");
		}
	}
	

//	*((uint16_t *) 0xB8002) = 0x7020;	
//	*/
/*
	int posx = 0;
	int posy = 0;
	char c = 'A';
	int state = 0;
	*/
	
#define RIGHT 0
#define DOWN 1
#define LEFT 2
#define UP 3
	while (1) {
//		put_char(posx, posy, c);
	//	uint16_t* addr = VIDEO_ADDR(posx, posy);
	//	*addr = (0x70 << 8) | c;
/*		switch (state)
		{
			case RIGHT:
				posx++;
				if (posx >= SCREEN_X_SIZE)
				{
					posx = SCREEN_X_SIZE-1;
					state = DOWN;
				}
				break;
			case DOWN:
				posy++;
				if (posy >= SCREEN_Y_SIZE)
				{
					posy = SCREEN_Y_SIZE-1;
					state = LEFT;
				}
				break;
			case LEFT:
				posx--;
				if (posx < 0)
				{
					posx = 0;
					state = UP;
				}
				break;
			case UP:
				posy--;
				if (posy < 0)
				{
					posy = 0;
					state = RIGHT;
					++c;
					if (c > 'Z')
					{
						c = 'A';
					}
				}
				break;
		}
		*/
		
		asm volatile("hlt"); // Do nothing...
	}

	// Probably don't want to return as we're doubling
	// as the idle process!
	return 0;
}

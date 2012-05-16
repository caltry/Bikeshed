/* This file contains the init process!
*/

typedef unsigned short uint16_t;
/*

void put_string(int x, int y, const char* c)
{
	while (*c != 0)
	{
		put_char(x, y, *c);

		++x;
		if (x > 80)
		{
			x = 0;
			y++;
		}

		if (y > 20)
		{
			y = 0;
		}

		++c;
	}
}

void place_msg()
{

	*((uint16_t *) 0xB8000) = 0x7020;	
}
*/

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

int main()
{
	//place_msg();
//	*((uint16_t *) 0xB8000) = 0x7020;	
	put_char(0, 0, 'W');
	put_char(1, 0, 'e');
	put_char(2, 0, 'l');
//	put_char(3, 0, 'c');
/*	put_char(4, 0, 'o');
	put_char(5, 0, 'm');
	put_char(6, 0, 'e');
	
//	*((uint16_t *) 0xB8002) = 0x7020;	
//	*/

	while (1) {
		asm volatile("hlt"); // Do nothing...
	}

	// Probably don't want to return as we're doubling
	// as the idle process!
	return 0;
}

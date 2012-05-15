/* This file contains the init process!
*/

typedef unsigned short uint16_t;

int main()
{
	*((uint16_t *) 0xB8000) = 0x7020;	

	while (1) {
		asm volatile("hlt"); // Do nothing...
	}

	// Probably don't want to return as we're doubling
	// as the idle process!
	return 0;
}

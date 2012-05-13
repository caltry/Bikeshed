/* This is a very simple user program
 * to verify that ELF loading works.
 *
 * It simply prints a character to the top
 * left of the screen
 */

typedef unsigned short uint16_t;

int main()
{
	*((uint16_t *) 0xB8000) = 0x7020;	

	return 0;
}

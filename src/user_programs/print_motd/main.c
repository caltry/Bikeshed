/*
 * Print the message of the day!
 *
 * Author: David C. Larsen <dcl9934@cs.rit.edu>, May 2012
 */

#include "ulib.h"

int main()
{
	Uint32 buffer_size = 1000;
	char buf[buffer_size];
	char filename[] = "/etc/motd";
	buf[0] = '\0';
	Uint32 bytes_read = 0;
	Uint32 offset = 0;
	Uint32 nbytes = buffer_size-1;

	do
	{
		fs_read( filename, buf, &bytes_read, offset, nbytes );
		buf[bytes_read] = 0;

		offset += bytes_read;

		for( int i = 0; i < bytes_read; ++i )
		{
			write( buf[i] );
		}
	} while( bytes_read );

	return 0;
}

/*
 * A collection of tests for Virtual File System functionality.
 *
 * Author: David Larsen <dcl9934@cs.rit.edu>, May 2012 
 */

#include "vfs.h"
#include "test_vfs.h"
#include "kernel/serial.h"

/*
 * Test basic read functionality.
 *
 * Nothing fancy, just read a few bytes of a file, starting with a small
 * offset.
 */
Uint32 test_raw_read(void);
Uint32 test_raw_read()
{
	Uint32 test_failures = 0;

	VFSStatus read_status;
	Uint32 buffer_size = 110;
	char buf[buffer_size];
	char filename[] = "/ext2_tests/indirect_block_file";
	Uint32 bytes_read;
	Uint32 offset = 10;

	buf[0] = '\0';
	read_status =
		raw_read (filename, buf, &bytes_read, offset, buffer_size-1);

	if( read_status )
	{
		serial_printf( "Failed to read '%s', error #%d\n\r",
		               filename, read_status );
		test_failures++;
	}

	buf[bytes_read] = '\0';
	serial_string ( "\n\r" );
	serial_string( buf );
	serial_string ( "\n\r" );

	return test_failures;
}

Uint32 vfs_run_tests()
{
	serial_string( "running VFS tests\n\r" );
	Uint32 test_failures = 0;
	test_failures += test_raw_read();

	serial_printf("==> VFS tests completed, %d failures.\n\r",
	              test_failures );

	return test_failures;
}

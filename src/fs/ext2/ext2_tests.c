/*
 * A collection of tests for various ext2 functionality (and bug reports).
 *
 * Author: David Larsen <dcl9934@cs.rit.edu>, May 2012
 */

#include "cpp_magic.h"
#include "read_ext2.h"
#include "kernel/serial.h"
#include "ext2_tests.h"

/*
 * Positive test cases for the lookup of different files.
 *
 * We use different file lengths here to make sure that calculations in
 * dirent_is_too_large correctly pad directory entry sizes. We need four
 * different files to test this, because we pad to 4 bytes.
 */
Uint32 test_file_lookup(void);
Uint32 test_file_lookup()
{
	ext2_read_status read_error;
	Uint bytes_read;
	char buf[1] = {'\0'};

	Uint32 test_failures = 0;
	char test_a[] = "/ext2_tests/a";
	char test_aa[] = "/ext2_tests/aa";
	char test_aaa[] = "/ext2_tests/aaa";
	char test_aaaa[] = "/ext2_tests/aaaa";
	char *test_files[] =
	{
		test_a,
		test_aa,
		test_aaa,
		test_aaaa,
		0
	};

	read_error = ext2_raw_read
		(bikeshed_ramdisk_context,
		 "/ext2_tests",
		 buf,
		 &bytes_read,
		 0,
		 0);
	if( read_error )
	{
		serial_printf( "Unable to read '/ext2_tests', all tests will "
		               "fail. Error #%d\n\r", read_error );
	}

	for(Uint i = 0; test_files[i]; i++)
	{
		read_error = ext2_raw_read
			(bikeshed_ramdisk_context,
			 test_files[i],
			 buf,
			 &bytes_read,
			 0,
			 0);
		if( read_error )
		{
			serial_printf( "Unable to read '%s' Error #%d\n\r",
					test_files[i], read_error );
			test_failures++;
		}
	}

	return test_failures;
}

/*
 * Test to make sure that we stop traversing for directory entries on time.
 *
 * This test is not automated, you'll have to look at the serial output to
 * determine if it stops scanning directories when it reaches the end of a
 * directory.
 */
Uint32 test_nonexistant_file_lookup(void);
Uint32 test_nonexistant_file_lookup()
{
	Uint32 test_failures = 0;

	ext2_read_status read_error;
	Uint bytes_read;
	char buf[1] = {'\0'};

	read_error = ext2_raw_read
		(bikeshed_ramdisk_context,
		 "/ext2_tests/nonexistant_file",
		 buf,
		 &bytes_read,
		 0,
		 0);

	if( !read_error )
	{
		serial_string
			( __FILE__ ":" CPP_STRINGIFY_RESULT(__LINE__) " "
			"We somehow read a non-existant file. This means we're"
			"not terminating directory traversals properly. (Or,"
			"someone created the file.");
		test_failures++;
	}

	return test_failures;
}

/*
 * Test our indirect block loading support.
 */
Uint32 test_indirect_block_reading(void);
Uint32 test_indirect_block_reading()
{
	Uint32 test_failures = 0;
	static const Uint32 buffer_size = 1026;

	ext2_read_status read_error;
	Uint bytes_read;
	char filename[] = "/ext2_tests/indirect_block_file";
	char buf[buffer_size];
	Uint lastLoc = 0;

	do{
		read_error = ext2_raw_read
			(bikeshed_ramdisk_context,
			 filename,
			 buf,
			 &bytes_read,
			 lastLoc,
			 buffer_size-1);

		lastLoc += bytes_read;

		if( read_error )
		{
			serial_printf
				( __FILE__ ":" CPP_STRINGIFY_RESULT(__LINE__) " "
				 "Unable to read '%s', error #%d\n\r",
				 filename, read_error );
			test_failures++;
			break;
		}
		buf[bytes_read] = '\0';
		serial_string("\n\r");
		serial_string(buf);
		serial_string("\n\r");
		buf[0] = '\0';
	}
	while( bytes_read && !read_error );

	return test_failures;
}

/*
 * Checks to make sure that we properly read a file at an offset. Only tests
 * offsets in the first block of a file.
 *
 * Currently, this is also a manual test, but eventually we want to make this
 * automated.
 *
 * Use a sliding window approach to ensure that what we're getting back is
 * consistent, so we re-read data often and use _strncmp to make sure that
 * we've read it correctly.
 */
Uint32 test_offset_read_small(void);
Uint32 test_offset_read_small()
{
	Uint32 test_failures = 0;
	static const Uint32 buffer_size = 9;

	ext2_read_status read_error;
	Uint bytes_read;
	char filename[] = "/ext2_tests/indirect_block_file";
	char buf[buffer_size];
	Uint lastLoc = 10;

	// TODO: actually do the sliding window thing.
	read_error = ext2_raw_read
		(bikeshed_ramdisk_context,
		filename,
		 buf,
		 &bytes_read,
		 lastLoc,
		 buffer_size-1);

	if( read_error )
	{
		serial_printf
			( __FILE__ ":" CPP_STRINGIFY_RESULT(__LINE__) " "
			 "Unable to read '%s', error #%d\n\r",
			 filename, read_error );
		test_failures++;
	}
	buf[bytes_read] = '\0';
	serial_string("\n\r");
	serial_string(buf);
	serial_string("\n\r");
	buf[0] = '\0';

	return test_failures;
}

/*
 * Checks to make sure that we properly read a file at an offset. Reads over a
 * block sizes' worth of data.
 *
 * Currently you need to manually observe this test to make sure that it's
 * passing. Eventually, the plan is that the sliding window tests should
 * automate all of that away.
 *
 * Use a sliding window approach to ensure that what we're getting back is
 * consistent, so we re-read data often and use _strncmp to make sure that
 * we've read it correctly.
 */
Uint32 test_offset_read_big(void);
Uint32 test_offset_read_big()
{
	Uint32 test_failures = 0;
	static const Uint32 buffer_size = 1111;

	ext2_read_status read_error;
	Uint bytes_read;
	char filename[] = "/ext2_tests/indirect_block_file";
	char buf[buffer_size];
	Uint lastLoc = 10;

	// TODO: actually do the sliding window thing.
	read_error = ext2_raw_read
		(bikeshed_ramdisk_context,
		filename,
		 buf,
		 &bytes_read,
		 lastLoc,
		 buffer_size-1);

	if( read_error )
	{
		serial_printf
			( __FILE__ ":" CPP_STRINGIFY_RESULT(__LINE__) " "
			 "Unable to read '%s', error #%d\n\r",
			 filename, read_error );
		test_failures++;
	}
	buf[bytes_read] = '\0';
	serial_string("\n\r");
	serial_string(buf);
	serial_string("\n\r");
	buf[0] = '\0';

	return test_failures;
}


/*
 * Try reading a file located at the root of the filesystem.
 *
 * We were having problems earlier where the directory traversal would only
 * search for files in subdirectories.
 */
Uint32 test_read_root_file(void);
Uint32 test_read_root_file()
{
	Uint32 test_failures = 0;
	static const Uint32 buffer_size = 1111;

	ext2_read_status read_error;
	Uint bytes_read;
	char filename[] = "/test.txt";
	char buf[buffer_size];
	Uint lastLoc = 0;

	read_error = ext2_raw_read
		(bikeshed_ramdisk_context,
		filename,
		buf,
		&bytes_read,
		lastLoc,
		buffer_size-1);

	if( read_error )
	{
		serial_printf
			( __FILE__ ":" CPP_STRINGIFY_RESULT(__LINE__) " "
			 "Unable to read '%s', error #%d\n\r",
			 filename, read_error );
		test_failures++;
	}
	buf[bytes_read] = '\0';
	serial_string("\n\r");
	serial_string(buf);
	serial_string("\n\r");
	buf[0] = '\0';

	return test_failures;
}

Uint32 test_all()
{
	Uint32 failed_tests = 0;
	failed_tests += test_nonexistant_file_lookup();
	failed_tests += test_file_lookup();
	failed_tests += test_indirect_block_reading();
	failed_tests += test_offset_read_small();
	failed_tests += test_offset_read_big();
	failed_tests += test_read_root_file();

	return failed_tests;
}

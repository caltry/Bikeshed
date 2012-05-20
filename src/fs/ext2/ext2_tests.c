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

Uint32 test_all()
{
	Uint32 failed_tests = 0;
	failed_tests += test_nonexistant_file_lookup();
	failed_tests += test_file_lookup();

	return failed_tests;
}

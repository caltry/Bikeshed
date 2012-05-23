/*
 * A collection of tests for various ext2 functionality (and bug reports).
 *
 * Author: David Larsen <dcl9934@cs.rit.edu>, May 2012
 */

#include "cpp_magic.h"
#include "read_ext2.h"
#include "write_ext2.h"
#include "kernel/serial.h"
#include "ext2_tests.h"

#define PRINT_NAME( test_name ) serial_string( "=> Test: " #test_name "\n\r" )

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
	PRINT_NAME( test_file_lookup );

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
	PRINT_NAME( test_nonexistant_file_lookup );
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
	PRINT_NAME( test_indirect_block_reading );
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
	PRINT_NAME( test_offset_read_small );
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
	PRINT_NAME( test_offset_read_big );
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
	PRINT_NAME( test_read_root_file );
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


/*
 * Make sure that we properly indicate when we can't find a file.
 */
Uint32 test_read_nonexistant_file(void);
Uint32 test_read_nonexistant_file()
{
	PRINT_NAME( test_read_nonexistant_file );
	Uint32 test_failures = 0;
	ext2_read_status read_error;

	static const Uint32 buffer_size = 2;
	Uint bytes_read;
	char filename[] = "/a_fake_file";
	char buf[buffer_size];
	Uint lastLoc = 0;

	// Try reading a non-existent file in the root directory
	read_error = ext2_raw_read
		(bikeshed_ramdisk_context,
		filename,
		buf,
		&bytes_read,
		lastLoc,
		buffer_size-1);

	switch( read_error )
	{
		case EXT2_READ_FILE_NOT_FOUND:
			break;
		default:
			serial_printf( __FILE__ ":"
			              CPP_STRINGIFY_RESULT(__LINE__) " "
				      "Got wrong error message while reading "
				      "'%s'. Error #%d\n\r",
				      filename,
				      read_error );
			test_failures++;
	}

	char filename2[] = "/ext2_tests/a_fake_file";

	// Try reading a non-existant file in a subdirectory
	read_error = ext2_raw_read
		(bikeshed_ramdisk_context,
		filename2,
		buf,
		&bytes_read,
		lastLoc,
		buffer_size-1);

	switch( read_error )
	{
		case EXT2_READ_FILE_NOT_FOUND:
			break;
		default:
			serial_printf( __FILE__ ":"
			              CPP_STRINGIFY_RESULT(__LINE__) " "
				      "Got wrong error message while reading "
				      "'%s'. Error #%d\n\r",
				      filename2,
				      read_error );
			test_failures++;
	}

	return test_failures;
}

/*
 * Print out a table version of the block bitmap. You'll have to compare this
 * table to the output from dumpe2fs.
 */
Uint32 test_block_is_occupied()
{
	PRINT_NAME( test_block_is_occupied );
	Uint32 test_failures = 0;

	for( int i = 0; i < 200; ++i )
	{
		serial_printf("Block %d: %d\t", i, block_is_occupied(bikeshed_ramdisk_context, i));
		++i;
		serial_printf("Block %d: %d\t", i, block_is_occupied(bikeshed_ramdisk_context, i));
		++i;
		serial_printf("Block %d: %d\n\r", i, block_is_occupied(bikeshed_ramdisk_context, i));
	}

	return test_failures;
}

/*
 * Tests to make sure that we can properly mark blocks in the block bitmap.
 *
 * Finds an unmarked block, marks it as occupied and does some basic sanity
 * checks.
 */
Uint32 test_mark_block_occupied()
{
	PRINT_NAME( test_mark_block_occupied );
	Uint32 test_failures = 0;

	struct ext2_filesystem_context *context = bikeshed_ramdisk_context;
	Uint32 sb_init_free_blocks =
		bikeshed_ramdisk_context->sb->free_blocks_count;
	Uint32 current_free_blocks = 0;
	
	for(Uint32 i = 0; i < bikeshed_ramdisk_context->sb->blocks_count; ++i)
	{
		if( !block_is_occupied( context, i ) )
		{
			if( block_is_occupied( context, i+1 ) )
			{
				test_failures++;
				serial_printf("Unexpected non-free block: %d, "
				              "It's unlikely that there is a "
					      "free block here, because it's "
					      "sourrounded by occuped blocks. "
					      "\n\r",
					      i );
			}
			serial_printf( "Marking block: %d\n\r", i );
			mark_block_occupied( context, i );
			if( !block_is_occupied( context, i ) )
			{
				test_failures++;
				serial_string( "Failed to mark the block "
				               "as occupied.\n\r" );
			}
			current_free_blocks =
				bikeshed_ramdisk_context->sb->free_blocks_count;
			if( current_free_blocks != sb_init_free_blocks -1 )
			{
				test_failures++;
				serial_string( "Number of free blocks did not "
				               "decrease.\n\r" );
			}
			break;
		}
	}

	return test_failures;
}


/*
 * Writes to an existing file which does not need to have more blocks allocated
 * to it for writing.
 */
Uint32 test_write_no_alloc(void);
Uint32 test_write_no_alloc()
{
	PRINT_NAME( test_write_no_alloc );
	Uint32 failed_tests = 0;

	char filename[] = "/ext2_tests/indirect_block_file";
	char buf[] = "test_write_no_alloc was here!";
	Uint32 bytes_read = 0;
	Uint32 offset = 0;
	Uint32 nbytes = sizeof(buf)-1;	// Exclude '\0'

	serial_printf( "going to write %d bytes to '%s'\n\r", nbytes, filename);

	ext2_write_status write_error = ext2_raw_write
		(bikeshed_ramdisk_context,
		filename,
		buf,
		&bytes_read,
		offset,
		nbytes );

	serial_printf( "wrote %d bytes to '%s'\n\r", bytes_read, filename);

	if( write_error )
	{
		serial_printf( "Unable to write to '%s', error #%d\n\r",
		               filename, write_error );
		failed_tests++;
	}

	char buf2[nbytes+10];
	buf2[0] = '\0';

	ext2_raw_read( bikeshed_ramdisk_context, filename, buf2, &bytes_read, offset, nbytes+8 );

	buf2[nbytes+9] = '\0';

	serial_string( buf2 );
	serial_string ("\n\r");

	return failed_tests;
}

/*
 * Writes to an empty file which needs to have blocks allocated to it, but
 * doesn't require allocating more than 12 blocks, so that we don't use
 * indirect blocks in the inode.
 */
Uint32 test_write_alloc_direct_blocks(void);
Uint32 test_write_alloc_direct_blocks()
{
	PRINT_NAME( test_write_alloc_direct_blocks );
	Uint32 failed_tests = 0;

	char filename[] = "/ext2_tests/a";
	char buf[] = "test_write_no_alloc was here!";
	Uint32 bytes_read = 0;
	Uint32 offset = 0;
	Uint32 nbytes = sizeof(buf)-1;	// Exclude '\0'

	serial_printf( "going to write %d bytes to '%s'\n\r", nbytes, filename);

	ext2_write_status write_error = ext2_raw_write
		(bikeshed_ramdisk_context,
		filename,
		buf,
		&bytes_read,
		offset,
		nbytes );

	serial_printf( "wrote %d bytes to '%s'\n\r", bytes_read, filename);

	if( write_error )
	{
		serial_printf( "Unable to write to '%s', error #%d\n\r",
		               filename, write_error );
		failed_tests++;
	}

	char buf2[nbytes+10];
	buf2[0] = '\0';

	ext2_raw_read( bikeshed_ramdisk_context, filename, buf2, &bytes_read, offset, nbytes+8 );

	buf2[nbytes+9] = '\0';

	serial_string( buf2 );
	serial_string ("\n\r");

	return failed_tests;
}


/*
 * Writes to an empty file which needs to have blocks allocated to it. Makes
 * use more than 12 blocks, so we need proper indirect block support.
 */
Uint32 test_write_alloc_indirect_blocks(void);
Uint32 test_write_alloc_indirect_blocks()
{
	PRINT_NAME( test_write_alloc_indirect_blocks );
	//TODO
	return 1;
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
	failed_tests += test_read_nonexistant_file();

	// Writing tests
	failed_tests += test_block_is_occupied();
	failed_tests += test_mark_block_occupied();
	failed_tests += test_write_no_alloc();
	failed_tests += test_write_alloc_direct_blocks();

	return failed_tests;
}

/*
 * Procedures for reading from ext2 filesystems
 *
 * David Larsen <dcl9934@cs.rit.edu>, April 2012
 */

#include "ext2.h"
#include "types.h"
#include "read_ext2.h"
#include "ext2_tests.h"
#include "kernel/serial.h"
#include "kernel/memory/kmalloc.h"
#include "kernel/lib/klib.h"
#include "kernel/lib/string.h"
#include "cpp_magic.h"
#include "common_ext2.h"

#define true 1
#define false 0

#ifndef RAMDISK_VIRT_LOCATION
#error "RAMDISK_VIRT_LOCATION not set!"
#endif
#ifndef RAMDISK_PHYS_LOCATION
#error "RAMDISK_PHYS_LOCATION not set!"
#endif

// Should we print a deluge of debugging information?
#ifndef DEBUG_FILESYSTEM
#define DEBUG_FILESYSTEM false
#endif

// Should we run a bunch of tests on the filesystem?
#ifndef EXT2_RUN_TESTS
#define EXT2_RUN_TESTS false
#endif

static const char *NEWLINE = "\n\r";

/*
 * The location of the inital ramdisk for Bikeshed.
 */
struct ext2_filesystem_context *bikeshed_ramdisk_context;

/*
 * Private helper function definitions.
 */
void print_file_data(struct ext2_filesystem_context *context, const char *path);

struct ext2_inode*
get_inode( struct ext2_filesystem_context *context, Uint32 inode_number );

/*
 * Find the inode number of a file, given the inode of the directory that it's
 * in.
 */
struct ext2_directory_entry*
get_file_from_dir_inode( struct ext2_filesystem_context *context,
			struct ext2_inode *directory_ino,
			const char *filename );

Uint32 ext2_read_file_by_inode
	(struct ext2_filesystem_context *context,
	struct ext2_directory_entry *file,
	void *buffer,
	Uint start,
	Uint nbytes );

struct ext2_directory_entry*
get_file_from_dir_path( struct ext2_filesystem_context *context,
			const char *dirpath,
			const char *filename );

void _fs_ext2_init()
{
	bikeshed_ramdisk_context = __kmalloc( sizeof( struct ext2_filesystem_context ) );
	bikeshed_ramdisk_context->sb = get_superblock( RAMDISK_VIRT_LOCATION );
	bikeshed_ramdisk_context->base_address = (Uint32) RAMDISK_VIRT_LOCATION;
	ext2_debug_dump( bikeshed_ramdisk_context );
}

void ext2_debug_dump( struct ext2_filesystem_context *context_ptr )
{
	struct ext2_superblock *sb = context_ptr->sb;
	serial_printf( "Dumping superblock located at: %x\n\r", (long) sb );
	print_superblock_data( sb );

	struct ext2_filesystem_context context = *context_ptr;

	serial_string("==\n\r");
	print_file_data( &context, "/" );
	serial_string("== get_dir_ents_root ==\n\r");
	print_dir_ents_root( &context );
	serial_string("==\n\r");

#if EXT2_RUN_TESTS
	serial_string("==Running ext2 tests==\n\r");
	Uint32 failures = test_all();
	serial_printf("Ext2 tests completed with: %d failures.\n\r", failures );
	serial_string("==Ending ext2 tests==\n\r");
#endif

	serial_string("== Printing out the message of the day! ==\n\r");
	char test_buffer[1112];
	ext2_read_status read_error;
	read_error = ext2_raw_read( &context, "/etc/motd", test_buffer, 0, 0, 100 );
	if( read_error )
	{
		serial_printf( "Unable to read \'/etc/motd\', error# %d\n\r",
		               read_error );
		return;
	}
	read_error = ext2_raw_read(&context, "/etc/motd", test_buffer+100, 0, 100,1111-100);
	if( read_error )
	{
		serial_printf( "Unable to read \'/etc/motd\', error# %d\n\r",
		               read_error );
		return;
	}
	test_buffer[1111] = '\0';
	serial_string( test_buffer );
	serial_string("==\n\r");
}


struct ext2_superblock*
get_superblock( Uint32 filesystem_start )
{
	struct ext2_superblock *sb;
	serial_printf( "filesystem_start: %x\n\r", filesystem_start );
	Uint32 sb_start = filesystem_start + (EXT2_SUPERBLOCK_LOCATION);
	sb = (struct ext2_superblock*) sb_start;
	return sb;
}

void print_superblock_data(struct ext2_superblock *sb)
{
	serial_string("=== Printing Superblock Data ===\n\r");
	serial_string( "Volume name: " );
	serial_string( (char*) sb->volume_name );
	serial_string( NEWLINE );
	serial_printf("Block size = %d\n\r",
		(1024 << sb->logarithmic_block_size));
	serial_printf( "%d inodes, ", sb->inodes_count );
	serial_printf( "%d blocks\n\r", sb->blocks_count );
	serial_printf( "%d blocks reserved for the super user\n\r",
		sb->reserved_blocks_count );
	serial_printf( "First data block: %d\n\r", sb->first_data_block );
	serial_printf( "First inode: %d\n\r", sb->first_inode );
	serial_printf( "Magic number: %x\n\r", sb->magic_number );
	serial_printf( "sb->features_read_only_nonbreaking %x \n\r",
			sb->features_read_only_nonbreaking );
	serial_string("=== END Printing Superblock Data ===\n\r");
	return;
}

void print_directory_entry_data(struct ext2_directory_entry *dirent)
{
	serial_string( "Filename: " );
	for( unsigned int i = 0; i < dirent->name_length; ++i )
	{
		serial_char( dirent->filename[i] );
	}
	serial_string( NEWLINE );
	serial_printf( "Inode: %d\n\r", dirent->inode_number );
	serial_printf( "Entry length: %d\n\r", dirent->entry_length);
	serial_printf( "Name length: %d\n\r", dirent->name_length );
	serial_printf( "File type: %d\n\r", dirent->file_type );
}

#define PRINT_VARIABLE( var ) serial_printf( #var ": %d\n\r", var )

void print_inode_data(struct ext2_inode *inode)
{
	serial_printf( "inode->mode: %x\n\r", inode->mode );
	PRINT_VARIABLE( inode->uid );
	PRINT_VARIABLE( inode->size );
	PRINT_VARIABLE( inode->atime );
	PRINT_VARIABLE( inode->mtime );
	PRINT_VARIABLE( inode->dtime );
	PRINT_VARIABLE( inode->gid );
	PRINT_VARIABLE( inode->incident_links );
	PRINT_VARIABLE( inode->num_blocks );
}

struct ext2_directory_entry* get_first_directory_entry
	(struct ext2_filesystem_context *context, struct ext2_inode* dir_file)
{
	serial_string( __FILE__ ":" CPP_STRINGIFY_RESULT(__LINE__) "\n\r" );
	serial_printf("dir_file: %x\n\r", (unsigned long) dir_file );

	void* dir_data_ptr = block_number_to_address( context, dir_file->blocks[0] );

	serial_string( __FILE__ ":" CPP_STRINGIFY_RESULT(__LINE__) "\n\r" );
	serial_printf("dir_data_ptr: %x\n\r", (unsigned long) dir_data_ptr);
	struct ext2_directory_entry *first_file;

	// We can just cast this, because we know that the block size is larger
	// than the largest directory entry.
	first_file = (struct ext2_directory_entry*) dir_data_ptr;

	return first_file;
}

struct ext2_inode*
get_inode( struct ext2_filesystem_context *context, Uint32 inode_number )
{
	struct ext2_superblock *sb = context->sb;
	serial_printf( __FILE__ ":" CPP_STRINGIFY_RESULT(__LINE__)
			" context->sb: %x\n\r", (unsigned long) sb );

	serial_printf( "Feching inode %d\n\r", inode_number );

	if( inode_number > context->sb->inodes_count )
	{
		serial_printf( "Requested inode num: %d\n\r", inode_number );
		serial_printf( "Maximum inode num: %d\n\r", context->sb->inodes_count );
		_kpanic( "ext2", "Requested an out of bounds inode number!",
			BAD_PARAM );
	}

	Uint block_group = inode_number/sb->inodes_per_group;

	_Bool block_group_has_superblock;
	if( sb->features_read_only_nonbreaking & EXT2_FEATURE_RO_COMPAT_SPARSE_SUPER )
	{
		block_group_has_superblock = false;
		serial_printf( "sb->features_read_only_nonbreaking %x\n\r",
				sb->features_read_only_nonbreaking );
		_kpanic( "ext2", "sparse superblock support not handled"
			__FILE__ ":" CPP_STRINGIFY_RESULT(__LINE__),
			FEATURE_UNIMPLEMENTED );
	} else {
		block_group_has_superblock = true;
	}

	// TODO!!! We need to handle sparse superblocks
	// Find the offset of the inode table into the superblock
	Uint inode_table_offset;
	if( block_group_has_superblock )
	{
		// FIXME: use the block group descriptor to find this number.
		inode_table_offset = 3;
	} else {
		_kpanic( "ext2", "sparse superblock support not handled"
			__FILE__ ":" CPP_STRINGIFY_RESULT(__LINE__),
			FEATURE_UNIMPLEMENTED );
	}

	// Traverse to the correct block group.
	// Find the inode table in that block group.
	// The inode table is always the 3rd block in a block group
	void* inode_table_addr = (void*)
				( context->base_address
				+ EXT2_SUPERBLOCK_LOCATION
				+ EXT2_SUPERBLOCK_SIZE
				+ inode_table_offset * get_block_size(sb)
				+ block_group * sb->blocks_per_group
				);

	serial_printf("inode_table: %x\n\r", (unsigned long) inode_table_addr);

	// What if the inode table is just a bunch of
	// inodes slammed together, rather than pointers to data blocks...?
	struct ext2_inode *our_inode = inode_table_addr
				+ (inode_number-1) * sizeof(struct ext2_inode);

	return our_inode;
}

void print_file_data(struct ext2_filesystem_context *context, const char *path)
{
	serial_string( "Printing file data for: " );
	serial_string( path );
	serial_string( "\n\r" );

	struct ext2_superblock *sb = context->sb;
	serial_printf( "sb: %x\n\r", (unsigned long) sb );

	struct ext2_inode *current_inode = get_inode( context, EXT2_INODE_ROOT );

	struct ext2_directory_entry *current_dir_entry;
	const char *current_path = path;

	serial_printf( "current_inode: %x\n\r", (unsigned long) current_inode );
	print_inode_data( current_inode );

	current_dir_entry = get_first_directory_entry( context, current_inode );

	// Ignore the beginning slash, we requre the path to be fully qualified
	// anyways.
	if (current_path[0] == '/')
	{
		current_path++;
	}

	if( current_path[0] == '.' )
	{
		if( current_path[1] == '.' )
		{
			// Parent directory
			print_directory_entry_data( current_dir_entry );
		}
		else if( current_path[1] == 0 )
		{
			// Current directory
		}
		else
		{
			// We don't handle this yet! TODO
			serial_string( "print_file_data in " __FILE__ ":" CPP_STRINGIFY_RESULT(__LINE__) " unimplemented" );
		}
	}

	// We're already looking at the file we want, and it just happend
	// to be a directory.
	else if( current_path[0] == 0 )
	{
		serial_string( __FILE__ ":" CPP_STRINGIFY_RESULT(__LINE__) "\n\r" );
		serial_printf( "current_dir_entry: %x\n\r",
			(unsigned long) current_dir_entry );
		print_directory_entry_data( current_dir_entry );
		serial_string( __FILE__ ":" CPP_STRINGIFY_RESULT(__LINE__) "\n\r" );
		print_inode_data( (struct ext2_inode*) current_dir_entry->inode_number );
	}
	else
	{
		//TODO
		serial_string( "print_file_data in " __FILE__ ":" CPP_STRINGIFY_RESULT(__LINE__) " unimplemented" );
	}
}

void print_dir_ents_root( struct ext2_filesystem_context *context )
{
	struct ext2_inode *dir_file = get_inode(context, EXT2_INODE_ROOT);

	Uint32 block_size = get_block_size( context->sb );

	for
	(Uint32 inode_block_idx = 0;
	inode_block_idx < EXT2_INODE_TOTAL_BLOCKS;
	inode_block_idx++)
	{
		// TODO: Support indirect, dub. indirect, trip. indirect blocks
		if( inode_block_idx >= EXT2_INODE_DIRECT_BLOCKS )
		{
			_kpanic( "ext2", "No support for indirect blocks",
				FEATURE_UNIMPLEMENTED );
		}

		Uint32 block_number = dir_file->blocks[inode_block_idx];
		struct ext2_directory_entry *file =
			(struct ext2_directory_entry*)
			block_number_to_address( context, block_number );
		struct ext2_directory_entry *first_file = file;

		/* Keep printing out entries until we've reached the end. */
		while( file->inode_number )
		{
			print_directory_entry_data( file );
			//file = ((void*) file) + file->entry_length;
			file = get_next_dirent( file );

			// If the next directory entry is in the next inode
			if( file > (first_file + block_size) )
			{
				break; // To the outer `for' loop
			}
		}

		// If we've reached this point, we either ran out of file
		// entires or just need to read the next block.
		if( file->inode_number )
		{
			continue; // To the next block
		} else {
			break;	// We've seen all of the file entries.
		}
	}
}


/*
 * A proof of concept towards implementing read(). It's easier to just dump the
 * data to serial than actually repackage it for now!
 *
 * For an ext2 _context_, read the file given by its _inode_num_ and print out
 * _nbytes_ of it.
 *
 * Returns the number of bytes read.
 */
Uint32
ext2_read_file_by_inode
	(struct ext2_filesystem_context *context,
	struct ext2_directory_entry *file,
	void *buffer,
	Uint start,
	Uint nbytes )
{
	Uint32 inode_num = file->inode_number;
	struct ext2_inode *fp = get_inode( context, inode_num );

	// Address space of a direct block
	Uint32 block_size = get_block_size( context->sb );
	// Address space of indirect blockss
	Uint32 indirect_block_size = block_size * block_size / sizeof(Uint32);
	Uint32 dub_indirect_block_size = indirect_block_size * indirect_block_size / sizeof(Uint32);
	Uint32 trip_indirect_block_size = dub_indirect_block_size * dub_indirect_block_size / sizeof(Uint32);

	// Avoid reading off of the end of a file.
	if( fp->size < (nbytes+start) )
	{
		// Don't try to start reading past the end of the file!
		if( start > fp->size )
		{
			nbytes = 0;
		} else {
			nbytes = fp->size - start;
		}
	}
	Uint remaining_bytes = nbytes;
	serial_printf( "going to read %d bytes\n\r", nbytes );

	// The number of blocks that we need to skip before reading
	// Logical inode block
	Uint32 first_inode_block = start / block_size;
	PRINT_VARIABLE( start );
	PRINT_VARIABLE( first_inode_block );

	// Literal inode block indexes
	Uint32 trip_indirect_inode_block_idx = 1;
	Uint32 dub_indirect_inode_block_idx = 1;
	Uint32 indirect_inode_block_idx;

	// FIXME, a bit of a hack to get indirect blocks working
	Uint32 direct_inode_block_idx;
	if( first_inode_block >= EXT2_INODE_INDIRECT_BLOCK_IDX )
	{
		direct_inode_block_idx = EXT2_INODE_INDIRECT_BLOCK_IDX;
		indirect_inode_block_idx = first_inode_block - EXT2_INODE_INDIRECT_BLOCK_IDX;
	} else {
		direct_inode_block_idx = first_inode_block;
		indirect_inode_block_idx = 0;
	}

	// The number of bytes into the first block that we need to skip
	Uint32 block_offset = start % block_size;

	for
	(Uint32 inode_block_idx = direct_inode_block_idx;
	inode_block_idx < EXT2_INODE_TOTAL_BLOCKS;
	inode_block_idx++)
	{
		Uint32 block_number = fp->blocks[inode_block_idx];
		PRINT_VARIABLE( block_number );
		Uint32 direct_inode_block_idx = first_inode_block % EXT2_INODE_TOTAL_BLOCKS;
		const char *data = (const char*)
			(block_offset + ((void*)
			block_number_to_address( context, block_number )));

		// Pointer to an indirect block data area (can't be declared
		// inside of the `switch' statement.
		const void *indirect_block;


#if DEBUG_FILESYSTEM
		PRINT_VARIABLE( block_offset );
		serial_printf("inode_block_idx: %d\n\r", inode_block_idx );
		PRINT_VARIABLE( indirect_inode_block_idx );
		PRINT_VARIABLE( dub_indirect_inode_block_idx );
		PRINT_VARIABLE( trip_indirect_inode_block_idx );
#endif

		// TODO: Support indirect, dub. indirect, trip. indirect blocks
		switch( inode_block_idx )
		{
		case EXT2_INODE_TRIP_INDIRECT_BLOCK_IDX:
		case EXT2_INODE_DUB_INDIRECT_BLOCK_IDX:
			_kpanic( "ext2", __FILE__ ":" CPP_STRINGIFY_RESULT(__LINE__) " No support for double or tripple indirect blocks",
				FEATURE_UNIMPLEMENTED );
		case EXT2_INODE_INDIRECT_BLOCK_IDX:

			// Dont' factor in the block offset when getting the
			// indirect data block.
			indirect_block = (const void*)
				(block_number_to_address( context, block_number ));

			// Don't let the inode block index advance unless
			// we've used all of the blocks in the indirect data
			// block.
			if( indirect_inode_block_idx < indirect_data_block_size(context) )
			{
				--inode_block_idx;
			}

			data = (const void*)
			 block_number_to_address( context, ((Uint32*) indirect_block)[indirect_inode_block_idx] );
			data += block_offset;
			serial_printf("data: %x\n\r", data );
			++indirect_inode_block_idx;
		}

		/* Keep printing out entries until we've reached the end. */
		if( remaining_bytes < block_size )
		{
			_kmemcpy( buffer, data, remaining_bytes );

			// We've reached the end of the file. Time to leave.
			remaining_bytes = 0;
			break;
		} else {
			serial_string(__FILE__ ":" CPP_STRINGIFY_RESULT(__LINE__) "\n\r");
			_kmemcpy( buffer, data, block_size-block_offset );
			buffer += (block_size-block_offset);

			// There's more to read
			remaining_bytes -= (block_size-block_offset);
		}

		// Reset the block offset -- we only use it once!
		block_offset = 0;
	}

	serial_printf( "actually read: %d bytes\n\r", nbytes-remaining_bytes );

	return nbytes-remaining_bytes;
}

struct ext2_directory_entry*
get_file_from_dir_inode( struct ext2_filesystem_context *context,
			struct ext2_inode *directory_ino,
			const char *filename )
{
	Uint32 filename_len = _kstrlen( filename );
	Uint32 block_size = get_block_size( context->sb );

	serial_string( __FILE__ ":" CPP_STRINGIFY_RESULT(__LINE__) "\n\r");
	serial_string("get_file_from_dir_inode(");
	serial_string(filename);
	serial_string(")\n\r");

	for(	Uint32 inode_block_idx = 0;
		inode_block_idx < EXT2_INODE_TOTAL_BLOCKS;
		inode_block_idx++)
	{
		if( inode_block_idx >= EXT2_INODE_DIRECT_BLOCKS )
		{
			_kpanic( "ext2", "I don't support extended blocks yet!", FEATURE_UNIMPLEMENTED );
		}

		Uint32 block_number = directory_ino->blocks[inode_block_idx];

		struct ext2_directory_entry *dirent =
			(struct ext2_directory_entry*)
			block_number_to_address(context, block_number);
		struct ext2_directory_entry *first_dirent = dirent;

		/* Keep going until the inode number is zero */
		while( dirent->inode_number )
		{
#if DEBUG_FILESYSTEM
			serial_string( __FILE__ ":" CPP_STRINGIFY_RESULT(__LINE__)
				" trying file: ");
			for( unsigned int i = 0; i < dirent->name_length; ++i )
			{
				serial_char( dirent->filename[i] );
			}
			serial_printf( " <%d>", dirent->inode_number );
			serial_printf( ", with length: %d  ", dirent->name_length);
			serial_printf( "strncmp(): %d\n\r", _kstrncmp( filename, dirent->filename, dirent->name_length ));
			serial_printf( "dirent->entry_length: %d", dirent->entry_length );
			serial_printf( " correct length? %d\n\r", !dirent_is_too_large( dirent ) );
#endif

			// If this is the file we're looking for, we're done!
			if( filename_len == dirent->name_length &&
			    !_kstrncmp( filename, dirent->filename, dirent->name_length ) )
			{
				return dirent;
			} else {
				// If this entry is too large, we know we're at
				// the end of the list of files in the
				// directory.
				if( dirent_is_too_large( dirent ) )
				{
					return 0;
				}

				dirent = get_next_dirent( dirent );

				if( dirent > (first_dirent + block_size) )
				{
					break; // To outer `for' loop
				}
			}
		}

		// If we've reached this point, we either ran out of file
		// entires or just need to read the next block.
		if( dirent->inode_number )
		{
			continue; // To next block
		} else {
			break;	// We've seen all of the file entries.
		}
	}

	return 0;
}

struct ext2_directory_entry*
get_file_from_dir_path( struct ext2_filesystem_context *context,
			const char *dirpath,
			const char *filename )
{
	serial_printf("get_file_from_dir_path( %s, %s )\n\r", dirpath, filename);

	if( dirpath[0] == DIRECTORY_SEPARATOR )
	{
		dirpath++;
	}
	const char *slash = _kstrchr( dirpath, DIRECTORY_SEPARATOR );
	Uint dir_name_length = slash - dirpath;

	// Mutable dirpath (+1 for null character)
	char current_dir_name[_kstrlen(dirpath)+1];


	struct ext2_inode *root_inode;
	root_inode = get_inode(context, EXT2_INODE_ROOT );
	struct ext2_inode *current_inode = root_inode;

	// Make the current dirent root (/)
	struct ext2_directory_entry *current_dirent =
		get_file_from_dir_inode( context, current_inode, "." );

	// While we still have more subdirectories to traverse to
	while( *dirpath )
	{
		_kmemcpy( current_dir_name, dirpath, dir_name_length );
		current_dir_name[dir_name_length] = '\0';
		dirpath += dir_name_length+1;

		current_dirent = get_file_from_dir_inode( context, current_inode, current_dir_name );

		// We didn't find the file or directory we were looking for
		if( current_dirent == 0 )
		{
			return 0;
		}

		dir_name_length = _kstrchr(dirpath, DIRECTORY_SEPARATOR) - dirpath;

		serial_printf( "fetching new dir inode: %d\n\r",
				 current_dirent->inode_number );
		current_inode = get_inode(context, current_dirent->inode_number);
	}

	struct ext2_inode *dirent_inode;
	dirent_inode = get_inode( context, current_dirent->inode_number );
	return get_file_from_dir_inode( context, dirent_inode, filename );
}

Uint32
get_file_size(struct ext2_filesystem_context *context, const char *path )
{
	if( path[0] != DIRECTORY_SEPARATOR )
	{
		return EXT2_READ_NO_LEADING_SLASH;
	}

	// Split up the path into the filename and dirname components
	Uint path_length = _kstrlen( path );
	const char *filename = _kstrrchr( path, DIRECTORY_SEPARATOR ) + 1;

	// Set up the directory name, keep the trailing slash, ignore filename.
	char dirname[path_length+1];
	_kmemcpy( dirname, path, path_length+1 );
	((char *)_kstrrchr( dirname, DIRECTORY_SEPARATOR ))[1] = '\0';

	struct ext2_directory_entry* file =
		get_file_from_dir_path( context, dirname, filename );

	if( file )
	{
		struct ext2_inode *fp = get_inode(context, file->inode_number);
		if( fp )
		{
			return fp->size;
		} else {
			return 0;
		}
	} else {
		return 0;
	}
}

ext2_read_status
ext2_raw_read
	(struct ext2_filesystem_context *context,
	const char *path,
	void *buffer,
	Uint *bytes_read,
	Uint start,
	Uint nbytes)
{
	if( !bytes_read )
	{
		Uint throwaway = 0;
		bytes_read = &throwaway;
	}
	*bytes_read = 0;

	if( path[0] != DIRECTORY_SEPARATOR )
	{
		return EXT2_READ_NO_LEADING_SLASH;
	}

	// Split up the path into the filename and dirname components
	Uint path_length = _kstrlen( path );
	const char *filename = _kstrrchr( path, DIRECTORY_SEPARATOR ) + 1;

	// Set up the directory name, keep the trailing slash, ignore filename.
	char dirname[path_length+1];
	_kmemcpy( dirname, path, path_length+1 );
	serial_printf( __FILE__ ":" CPP_STRINGIFY_RESULT(__LINE__)
			", dirname before excluding filename: %s\n\r",
			dirname );
	((char *)_kstrrchr( dirname, DIRECTORY_SEPARATOR ))[1] = '\0';
	serial_printf( __FILE__ ":" CPP_STRINGIFY_RESULT(__LINE__)
			", dirname after excluding filename: %s\n\r",
			dirname );
	((char *)_kstrrchr( dirname, DIRECTORY_SEPARATOR ))[1] = '\0';

	struct ext2_directory_entry* file =
		get_file_from_dir_path( context, dirname, filename );

	if( file )
	{
		*bytes_read = ext2_read_file_by_inode(context, file, buffer, start, nbytes);
		return EXT2_READ_SUCCESS;
	} else {
		return EXT2_READ_FILE_NOT_FOUND;
	}
}

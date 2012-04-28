/*
 * Procedures for reading from ext2 filesystems
 *
 * David Larsen <dcl9934@cs.rit.edu>, April 2012
 */

#include "ext2.h"
#include "types.h"
#include "read_ext2.h"
#include "kernel/serial.h"
#include "kernel/lib/klib.h"
#include "cpp_magic.h"

#define true 1
#define false 0

static const char *NEWLINE = "\n\r";

/*
 * Private helper function definitions.
 */
void print_file_data(struct ext2_filesystem_context *context, const char *path);

inline Uint get_block_size( struct ext2_superblock *sb )
{
	return 1024 << sb->logarithmic_block_size;
}

inline void*
block_number_to_address( struct ext2_filesystem_context *context,
			Uint32 one_indexed_block_number )
{
	Uint32 block_number = one_indexed_block_number-1;

	return (void*) (context->base_address + EXT2_SUPERBLOCK_LOCATION)
		+ (block_number * get_block_size(context->sb));
}

void _fs_ext2_init()
{
	ext2_debug_dump();
}

void ext2_debug_dump()
{
	struct ext2_superblock *sb = get_superblock( TEMP_RAMDISK_LOCATION );
	serial_printf( "Dumping superblock located at: %x\n\r", (long) sb );
	print_superblock_data( sb );

	struct ext2_filesystem_context context = { sb, TEMP_RAMDISK_LOCATION };


	serial_string("==\n\r");
	print_file_data( &context, "/" );
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
	serial_string( dirent->filename );
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

	struct ext2_inode *current_inode = get_inode( context, sb->first_inode );

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

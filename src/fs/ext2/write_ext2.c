/*
 * Functions for writing to the ext2 filesystem.
 *
 * Author: David Larsen <dcl9934@cs.rit.edu>, May 2012
 */

#include "ext2.h"
#include "common_ext2.h"
#include "write_ext2.h"
#include "cpp_magic.h"
#include "kernel/lib/string.h"

#define PRINT_VARIABLE( var ) serial_printf( #var ": %d\n\r", var )

/*
 * Given a block number, find its block group number.
 */
static inline Uint32
block_to_block_group( struct ext2_filesystem_context *context, Uint32 block_number )
{
	return block_number / context->sb->blocks_per_group;
}

/*
 * Find the block bitmap for the block group which contains _block_number_.
 */
Uint32*
get_block_bitmap( struct ext2_filesystem_context *context, Uint32 block_number )
{
	static const Uint32 block_group_descriptor_table_block_number = 2;
	Uint32 block_group_number = block_to_block_group( context, block_number );

	struct ext2_block_group_descriptor *block_group_descriptor_table =
		block_number_to_address
			(context,
			block_group_descriptor_table_block_number );
	BlockNumber block_bitmap_block_number =
		block_group_descriptor_table[block_group_number].block_bitmap;

	Uint32 *block_bitmap = (Uint32*)
		block_number_to_address( context, block_bitmap_block_number );
	
	return block_bitmap;
}

/*
 * Checks to see if a block has been marked as occuped, according to the block
 * group's block bitmap.
 */
_Bool
block_is_occupied( struct ext2_filesystem_context *context, Uint32 block_number )
{
	Uint32 *block_bitmap = get_block_bitmap( context, block_number );
	Uint32 block_bitmap_index = (block_number%context->sb->blocks_per_group)
	                          / 32;
	
	return block_bitmap[block_bitmap_index] & 1 << (block_number%32);
}

/*
 * Marks a block as occupied. Does *not* check to see if the block has already
 * been marked as occupied.
 */
void
mark_block_occupied( struct ext2_filesystem_context *context, Uint32 block_number )
{
	Uint32 *block_bitmap = get_block_bitmap( context, block_number );
	
	Uint32 block_bitmap_index = (block_number%context->sb->blocks_per_group)
	                          / 32;
	
	block_bitmap[block_bitmap_index] |= 1 << (block_number%32);

	// Now update the free blocks count in superblock and block group
	// descriptor
	context->sb->free_blocks_count--;

	static const Uint32 block_group_descriptor_table_block_number = 2;
	Uint32 block_group_number = block_to_block_group( context, block_number );
	struct ext2_block_group_descriptor *block_group_descriptor_table =
		block_number_to_address
			(context,
			block_group_descriptor_table_block_number );
	block_group_descriptor_table[block_group_number].free_blocks_count--;
}


/*
 * Allocate the first available block.
 *
 * Returns: the block number or 0 on failure.
 */
BlockNumber
block_alloc( struct ext2_filesystem_context *context )
{
	// TODO, make this lookup take advantage of bitwise operations
	for(Uint32 i = 0; i < context->sb->blocks_count; i++)
	{
		if( !block_is_occupied( context, i ) )
		{
			mark_block_occupied( context, i );
			return i;
		}
	}

	// We've failed to find an unoccupied block.
	return 0;
}

/*
 * Add a number of blocks to an inode.
 */
Uint32
increase_inode_size
	(struct ext2_filesystem_context *context,
	struct ext2_inode *fp,
	Uint32 nbytes)
{
	Uint32 block_size = get_block_size( context->sb );
	Uint32 initial_size = fp->size;
	Uint32 logical_block_idx = initial_size / block_size;

	Uint32 blocks_allocated = 0;
	Uint32 total_bytes_to_alloc = nbytes;

	// Account for unused space in the last block.
	if( initial_size % block_size )
	{
		nbytes -= block_size - (initial_size % block_size);
	}

	// Allocate at least nbytes worth of blocks. Because it needs to be
	// aligned, we'll always round up.
	Uint32 num_blocks_to_alloc = nbytes / block_size;
	if( nbytes % block_size )
	{
		num_blocks_to_alloc += 1;
	}

	BlockNumber last_block_allocked = 0;
	for( Uint i = 0; i < num_blocks_to_alloc; ++i )
	{
		last_block_allocked = block_alloc( context );

		// Was the alloc successful?
		if( last_block_allocked )
		{
			blocks_allocated++;
			Uint32 *inode_block = get_inode_block( fp, logical_block_idx );
			*inode_block = last_block_allocked;
		} else {
			break;
		}
	}

	// Record the new size of the inode
	if( blocks_allocated == num_blocks_to_alloc )
	{
		fp->size += total_bytes_to_alloc;
	} else {
		fp->size += total_bytes_to_alloc -
			((num_blocks_to_alloc - blocks_allocated) * block_size);
	}

	return blocks_allocated;
}

Uint32
ext2_write_file_by_inode
	(struct ext2_filesystem_context *context,
	struct ext2_directory_entry *file,
	void *buffer,
	Uint32 start,
	Uint32 nbytes)
{
	Uint32 inode_num = file->inode_number;
	struct ext2_inode *fp = get_inode( context, inode_num );

	Uint32 block_size = get_block_size( context->sb );

	Uint32 indirect_block_size = block_size * block_size / sizeof(Uint32);

	// If we're going to write off of the end of a file, we need to
	// allocate a new block for the inode.
	if( fp->size < (nbytes+start) )
	{
		// We can't have a gap in our file; don't start writing past
		// the end of the file.
		if( start > fp->size )
		{
			return EXT2_WRITE_NON_CONTIGUOUS;
		} else {
			increase_inode_size
				( context,
				fp,
				nbytes+start-(fp->size) );
		}
	}

	// At this point, we know that we have enough blocks to store the data.
	Uint32 remaining_bytes = nbytes;

	// Skip ahead to the block that we need to write
	Uint32 first_inode_block = start / block_size;

	// Literal inode block indexes
	Uint32 indirect_inode_block_idx;
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

		// TODO: Support dub. indirect, trip. indirect blocks
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

		if( remaining_bytes < block_size )
		{
			_kmemcpy( data, buffer, remaining_bytes );

			// We've copied everything we need to. Time to leave.
			remaining_bytes = 0;
			break;
		} else {
			_kmemcpy( data, buffer, block_size-block_offset );
			buffer += (block_size-block_offset);

			// There's more to read
			remaining_bytes -= (block_size-block_offset);
		}

		// Reset the block offset -- we only use it once!
		block_offset = 0;
	}

	return nbytes-remaining_bytes;
}

ext2_write_status
ext2_raw_write
	(struct ext2_filesystem_context *context,
	const char *path,
	const void *buffer,
	Uint32 *bytes_written,
	Uint32 start,
	Uint32 nbytes)
{
	if( !bytes_written )
	{
		Uint32 throwaway = 0;
		bytes_written = &throwaway;
	}
	*bytes_written = 0;

	if( path[0] != DIRECTORY_SEPARATOR )
	{
		return EXT2_WRITE_NO_LEADING_SLASH;
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
		*bytes_written = ext2_write_file_by_inode(context, file, buffer, start, nbytes);
		return EXT2_WRITE_SUCCESS;
	}
	else
	{
		return EXT2_WRITE_FILE_NOT_FOUND;
	}
}

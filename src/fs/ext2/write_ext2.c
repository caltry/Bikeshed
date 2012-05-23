/*
 * Functions for writing to the ext2 filesystem.
 *
 * Author: David Larsen <dcl9934@cs.rit.edu>, May 2012
 */

#include "ext2.h"
#include "common_ext2.h"
#include "write_ext2.h"

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

// TODO
#if 0
ext2_write_status
ext2_raw_write
	(struct ext2_filesystem_context *context,
	const char *path,
	const void *buffer,
	Uint32 *bytes_written,
	Uint32 start,
	Uint32 nbytes)
{
	return -1;
}
#endif

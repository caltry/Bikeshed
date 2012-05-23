/*
 * Helper functions which have common use between reading and writing.
 *
 * Author: David C. Larsen <dcl9934@cs.rit.edu>, May 2012
 */

#ifndef _COMMON_EXT2_H
#define _COMMON_EXT2_H

static inline Uint get_block_size( struct ext2_superblock *sb )
{
	return 1024 << sb->logarithmic_block_size;
}

static inline void*
block_number_to_address( struct ext2_filesystem_context *context,
			Uint32 one_indexed_block_number )
{
	Uint32 block_number = one_indexed_block_number-1;

	return (void*) (context->base_address + EXT2_SUPERBLOCK_LOCATION)
		+ (block_number * get_block_size(context->sb));
}

/*
 * Get the next directory entry.
 *
 * This has *no* guarentee that there will actually be a directory entry, but
 * is more like an array++ operator. It's entirely possible that the dirent
 * returned isn't even in the same block. You should check for this.
 */
static inline struct ext2_directory_entry*
get_next_dirent( struct ext2_directory_entry *dirent )
{
	return ((void*) dirent) + dirent->entry_length;
}


/*
 * Determine if the size of this dirent is too large. This means that this is
 * the last file entry in the directory.
 */
static inline _Bool
dirent_is_too_large( struct ext2_directory_entry *dirent )
{
	Uint16 expected_size = sizeof( struct ext2_directory_entry )
	                     + dirent->name_length;
	// Pad size to 4 byte boundary
	if( expected_size % 4 )
	{
		expected_size += 4 - (expected_size % 4);
	}

	return expected_size < dirent->entry_length;
}

/*
 * Give the number of data blocks that a single indirect block can point to.
 */
static inline Uint32
indirect_data_block_size( struct ext2_filesystem_context *context )
{
	return get_block_size( context->sb ) / sizeof(Uint32);
}

#endif //_COMMON_EXT2_H

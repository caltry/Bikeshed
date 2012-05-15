/*
 * Runs a bunch of compile-time tests to do basic sanity checking.
 *
 * David Larsen <dcl9934@cs.rit.edu>, April 2012
 */
#include "ext2.h"
#include "cpp_magic.h"

/*
 * Check to make sure that the size of the disk structures are what the
 * specifications describe.
 */
void check_structure_sizes(void) __attribute__ ((unused));
void check_structure_sizes()
{
	COMPILE_ERROR_IF(sizeof(struct ext2_superblock) != EXT2_SUPERBLOCK_SIZE);
	COMPILE_ERROR_IF(sizeof(struct ext2_inode) != EXT2_INODE_SIZE);
	COMPILE_ERROR_IF(sizeof(struct ext2_block_group_descriptor)
		!= EXT2_BLOCK_GROUP_DESCRIPTOR_SIZE );
}

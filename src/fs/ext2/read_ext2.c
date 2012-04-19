/*
 * Procedures for reading from ext2 filesystems
 *
 * David Larsen <dcl9934@cs.rit.edu>, April 2012
 */

#include "ext2.h"
#include "read_ext2.h"
#include "kernel/serial.h"

void _fs_ext2_init()
{
	ext2_debug_dump();
}

void ext2_debug_dump()
{
	struct ext2_superblock *sb = get_superblock( TEMP_RAMDISK_LOCATION );
	serial_printf( "Dumping superblock located at: %x\n\r", sb );
	print_superblock_data( sb );
}


struct ext2_superblock*
get_superblock( Uint32 filesystem_start )
{
	struct ext2_superblock *sb;
	serial_printf( "filesystem_start: %x\n\r", filesystem_start );
	sb = filesystem_start + (EXT2_SUPERBLOCK_LOCATION);
}

/*
 * Dump interesting superblock data from the filesyste, to see if we're reading
 * it properly.
 *
 * Like the output from mke2fs:
 * Filesystem label=
 * OS type: Linux
 * Block size=1024 (log=0)
 * Fragment size=1024 (log=0)
 * Stride=0 blocks, Stripe width=0 blocks
 * 2048 inodes, 8192 blocks
 * 409 blocks (4.99%) reserved for the super user
 * First data block=1
 * Maximum filesystem blocks=8388608
 * 1 block group
 * 8192 blocks per group, 8192 fragments per group
 * 2048 inodes per group
 */
void print_superblock_data(struct ext2_superblock *sb)
{
	serial_string("=== Printing Superblock Data ===\n\r");
	serial_printf("Block size = %d\n\r",
		(1024 << sb->logarithmic_block_size));
	serial_printf( "%d inodes, ", sb->inodes_count );
	serial_printf( "%d blocks\n\r", sb->blocks_count );
	serial_printf( "%d blocks reserved for the super user\n\r",
		sb->reserved_blocks_count );
	serial_printf( "First data block: %d\n\r", sb->first_data_block );
	serial_printf( "Magic number: %x\n\r", sb->magic_number );
	serial_string("=== END Printing Superblock Data ===\n\r");
	return;
}

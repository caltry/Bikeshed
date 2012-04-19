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

void print_superblock_data(struct ext2_superblock *sb)
{
	serial_string("=== Printing Superblock Data ===\n\r");
	serial_string( "Volume name: " );
	serial_string( sb->volume_name );
	serial_string( "\n\r" );
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

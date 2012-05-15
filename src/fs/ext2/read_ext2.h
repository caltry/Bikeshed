#ifndef _READ_EXT2_H
#define _READ_EXT2_H

#include "types.h"
#include "ext2.h"

/*
 * The location of the inital ramdisk for Bikeshed.
 */
extern struct ext2_filesystem_context *bikeshed_ramdisk_context;

typedef enum {
	EXT2_READ_SUCCESS = 0,
	EXT2_READ_FILE_NOT_FOUND = 1,
	EXT2_READ_NO_LEADING_SLASH = 2
} ext2_read_status;

struct ext2_superblock *get_superblock( Uint32 filesystem_start );

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
void print_superblock_data(struct ext2_superblock *sb);

/*
 * Dump a bunch of debugging info to serial.
 */
void ext2_debug_dump( struct ext2_filesystem_context *context );


/*****************
 * File contents *
 *****************/

/*
 * For this mounted filesystem _context_, read _nbytes_ from _path_,
 * starting _start_ bytes into the file. Store the read data into the _buffer_.
 * Records the number of _bytes_read_.
 *
 * Returns a status:
 * 	EXT2_READ_SUCCESS if we successfully read >= 0 bytes.
 * 	EXT2_READ_FILE_NOT_FOUND if we couldn't find the requested file.
 * 	EXT2_READ_NO_LEADING_SLASH if the path doesn't include a fully
 * 	                           qualified directory name (e.g. "/text.txt")
 */
ext2_read_status
ext2_raw_read
	(struct ext2_filesystem_context *context, 
	const char *path,
	void *buffer,
	Uint *bytes_read,
	Uint start,
	Uint nbytes);


/***************
 * Directories *
 ***************/

/*
 * List the contents of the root directory.
 *
 * XXX: This function is being used as scaffolding and will eventually be
 * removed.
 */
void print_dir_ents_root( struct ext2_filesystem_context *context );

#endif // _READ_EXT2_H

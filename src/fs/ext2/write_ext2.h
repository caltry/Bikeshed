/*
 * Functions for writing to the ext2 filesystem.
 *
 * Author: David Larsen <dcl9934@cs.rit.edu>, May 2012
 */

#ifndef _WRITE_EXT2_H
#define _WRITE_EXT2_H

#include "types.h"

struct ext2_filesystem_context;

typedef enum {
	EXT2_WRITE_SUCCESS = 0
} ext2_write_status;

/*
 * For this mounted filesystem_context_, write _nbytes_ to _path_, starting
 * _start_ bytes into the file. Write the data from the source _buffer_.
 * Records the number of _bytes_written_.
 *
 * Returns a status:
 * 	EXT2_WRITE_SUCCESS If we successfully wrote >= 0 bytes.
 * 	EXT2_WRITE_FILE_NOT_FOUND if we couldn't find the requested file.
 * 	EXT2_NO_LEADING_SLASH if the path doesn't include a fully qualified
 * 	                      directory name (e.g. "/etc/motd")
 */
ext2_write_status
ext2_raw_write
	(struct ext2_filesystem_context *context,
	const char *path,
	const void *buffer,
	Uint32 *bytes_written,
	Uint32 start,
	Uint32 nbytes);

#endif //_WRITE_EXT2_H
